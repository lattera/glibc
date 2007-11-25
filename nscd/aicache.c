/* Cache handling for host lookup.
   Copyright (C) 2004, 2005, 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2004.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <assert.h>
#include <errno.h>
#include <libintl.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "dbg_log.h"
#include "nscd.h"
#ifdef HAVE_SENDFILE
# include <kernel-features.h>
#endif


typedef enum nss_status (*nss_gethostbyname3_r)
  (const char *name, int af, struct hostent *host,
   char *buffer, size_t buflen, int *errnop,
   int *h_errnop, int32_t *, char **);
typedef enum nss_status (*nss_getcanonname_r)
  (const char *name, char *buffer, size_t buflen, char **result,
   int *errnop, int *h_errnop);


static const ai_response_header notfound =
{
  .version = NSCD_VERSION,
  .found = 0,
  .naddrs = 0,
  .addrslen = 0,
  .canonlen = 0,
  .error = 0
};


static void
addhstaiX (struct database_dyn *db, int fd, request_header *req,
	   void *key, uid_t uid, struct hashentry *he, struct datahead *dh)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */

  /* We allocate all data in one memory block: the iov vector,
     the response header and the dataset itself.  */
  struct dataset
  {
    struct datahead head;
    ai_response_header resp;
    char strdata[0];
  } *dataset = NULL;

  if (__builtin_expect (debug_level > 0, 0))
    {
      if (he == NULL)
	dbg_log (_("Haven't found \"%s\" in hosts cache!"), (char *) key);
      else
	dbg_log (_("Reloading \"%s\" in hosts cache!"), (char *) key);
    }

  static service_user *hosts_database;
  service_user *nip = NULL;
  int no_more;
  int rc6 = 0;
  int rc4 = 0;
  int herrno = 0;

  if (hosts_database != NULL)
    {
      nip = hosts_database;
      no_more = 0;
    }
  else
    no_more = __nss_database_lookup ("hosts", NULL,
				     "dns [!UNAVAIL=return] files", &nip);

  if (__res_maybe_init (&_res, 0) == -1)
	    no_more = 1;

  /* If we are looking for both IPv4 and IPv6 address we don't want
     the lookup functions to automatically promote IPv4 addresses to
     IPv6 addresses.  Currently this is decided by setting the
     RES_USE_INET6 bit in _res.options.  */
  int old_res_options = _res.options;
  _res.options &= ~RES_USE_INET6;

  size_t tmpbuf6len = 512;
  char *tmpbuf6 = alloca (tmpbuf6len);
  size_t tmpbuf4len = 0;
  char *tmpbuf4 = NULL;
  char *canon = NULL;
  int32_t ttl = INT32_MAX;
  ssize_t total = 0;
  char *key_copy = NULL;
  bool alloca_used = false;

  while (!no_more)
    {
      int status[2] = { NSS_STATUS_UNAVAIL, NSS_STATUS_UNAVAIL };

      /* Prefer the function which also returns the TTL and canonical name.  */
      nss_gethostbyname3_r fct = __nss_lookup_function (nip,
							"gethostbyname3_r");
      if (fct == NULL)
	fct = __nss_lookup_function (nip, "gethostbyname2_r");

      if (fct != NULL)
	{
	  struct hostent th[2];

	  /* Collect IPv6 information first.  */
	  while (1)
	    {
	      rc6 = 0;
	      status[0] = DL_CALL_FCT (fct, (key, AF_INET6, &th[0], tmpbuf6,
					     tmpbuf6len, &rc6, &herrno,
					     &ttl, &canon));
	      if (rc6 != ERANGE || herrno != NETDB_INTERNAL)
		break;
	      tmpbuf6 = extend_alloca (tmpbuf6, tmpbuf6len, 2 * tmpbuf6len);
	    }

	  if (rc6 != 0 && herrno == NETDB_INTERNAL)
	    goto out;

	  /* If the IPv6 lookup has been successful do not use the
	     buffer used in that lookup, use a new one.  */
	  if (status[0] == NSS_STATUS_SUCCESS && rc6 == 0)
	    {
	      tmpbuf4len = 512;
	      tmpbuf4 = alloca (tmpbuf4len);
	    }
	  else
	    {
	      tmpbuf4len = tmpbuf6len;
	      tmpbuf4 = tmpbuf6;
	    }

	  /* Next collect IPv4 information.  */
	  while (1)
	    {
	      rc4 = 0;
	      status[1] = DL_CALL_FCT (fct, (key, AF_INET, &th[1], tmpbuf4,
					     tmpbuf4len, &rc4, &herrno,
					     ttl == INT32_MAX ? &ttl : NULL,
					     canon == NULL ? &canon : NULL));
	      if (rc4 != ERANGE || herrno != NETDB_INTERNAL)
		break;
	      tmpbuf4 = extend_alloca (tmpbuf4, tmpbuf4len, 2 * tmpbuf4len);
	    }

	  if (rc4 != 0 && herrno == NETDB_INTERNAL)
	    goto out;

	  if (status[0] == NSS_STATUS_SUCCESS
	      || status[1] == NSS_STATUS_SUCCESS)
	    {
	      /* We found the data.  Count the addresses and the size.  */
	      int naddrs = 0;
	      size_t addrslen = 0;
	      for (int j = 0; j < 2; ++j)
		if (status[j] == NSS_STATUS_SUCCESS)
		  for (int i = 0; th[j].h_addr_list[i] != NULL; ++i)
		    {
		      ++naddrs;
		      addrslen += th[j].h_length;
		    }

	      if (canon == NULL)
		{
		  /* Determine the canonical name.  */
		  nss_getcanonname_r cfct;
		  cfct = __nss_lookup_function (nip, "getcanonname_r");
		  if (cfct != NULL)
		    {
		      const size_t max_fqdn_len = 256;
		      char *buf = alloca (max_fqdn_len);
		      char *s;
		      int rc;

		      if (DL_CALL_FCT (cfct, (key, buf, max_fqdn_len, &s, &rc,
					      &herrno)) == NSS_STATUS_SUCCESS)
			canon = s;
		      else
			/* Set to name now to avoid using gethostbyaddr.  */
			canon = key;
		    }
		  else
		    {
		      struct hostent *he = NULL;
		      int herrno;
		      struct hostent he_mem;
		      void *addr;
		      size_t addrlen;
		      int addrfamily;

		      if (status[1] == NSS_STATUS_SUCCESS)
			{
			  addr = th[1].h_addr_list[0];
			  addrlen = sizeof (struct in_addr);
			  addrfamily = AF_INET;
			}
		      else
			{
			  addr = th[0].h_addr_list[0];
			  addrlen = sizeof (struct in6_addr);
			  addrfamily = AF_INET6;
			}

		      size_t tmpbuflen = 512;
		      char *tmpbuf = alloca (tmpbuflen);
		      int rc;
		      while (1)
			{
			  rc = __gethostbyaddr2_r (addr, addrlen, addrfamily,
						   &he_mem, tmpbuf, tmpbuflen,
						   &he, &herrno, NULL);
			  if (rc != ERANGE || herrno != NETDB_INTERNAL)
			    break;
			  tmpbuf = extend_alloca (tmpbuf, tmpbuflen,
						  tmpbuflen * 2);
			}

		      if (rc == 0)
			{
			  if (he != NULL)
			    canon = he->h_name;
			  else
			    canon = key;
			}
		    }
		}
	      size_t canonlen = canon == NULL ? 0 : (strlen (canon) + 1);

	      total = sizeof (*dataset) + naddrs + addrslen + canonlen;

	      /* Now we can allocate the data structure.  If the TTL
		 of the entry is reported as zero do not cache the
		 entry at all.  */
	      if (ttl != 0 && he == NULL)
		{
		  dataset = (struct dataset *) mempool_alloc (db,
							      total
							      + req->key_len);
		  if (dataset == NULL)
		    ++db->head->addfailed;
		}

	      if (dataset == NULL)
		{
		  /* We cannot permanently add the result in the moment.  But
		     we can provide the result as is.  Store the data in some
		     temporary memory.  */
		  dataset = (struct dataset *) alloca (total + req->key_len);

		  /* We cannot add this record to the permanent database.  */
		  alloca_used = true;
		}

	      dataset->head.allocsize = total + req->key_len;
	      dataset->head.recsize = total - offsetof (struct dataset, resp);
	      dataset->head.notfound = false;
	      dataset->head.nreloads = he == NULL ? 0 : (dh->nreloads + 1);
	      dataset->head.usable = true;

	      /* Compute the timeout time.  */
	      dataset->head.timeout = time (NULL) + (ttl == INT32_MAX
						     ? db->postimeout : ttl);

	      dataset->resp.version = NSCD_VERSION;
	      dataset->resp.found = 1;
	      dataset->resp.naddrs = naddrs;
	      dataset->resp.addrslen = addrslen;
	      dataset->resp.canonlen = canonlen;
	      dataset->resp.error = NETDB_SUCCESS;

	      char *addrs = (char *) (&dataset->resp + 1);
	      uint8_t *family = (uint8_t *) (addrs + addrslen);

	      for (int j = 0; j < 2; ++j)
		if (status[j] == NSS_STATUS_SUCCESS)
		  for (int i = 0; th[j].h_addr_list[i] != NULL; ++i)
		    {
		      addrs = mempcpy (addrs, th[j].h_addr_list[i],
				       th[j].h_length);
		      *family++ = th[j].h_addrtype;
		    }

	      void *cp = family;
	      if (canon != NULL)
		cp = mempcpy (cp, canon, canonlen);

	      key_copy = memcpy (cp, key, req->key_len);

	      /* Now we can determine whether on refill we have to
		 create a new record or not.  */
	      if (he != NULL)
		{
		  assert (fd == -1);

		  if (total + req->key_len == dh->allocsize
		      && total - offsetof (struct dataset, resp) == dh->recsize
		      && memcmp (&dataset->resp, dh->data,
				 dh->allocsize
				 - offsetof (struct dataset, resp)) == 0)
		    {
		      /* The data has not changed.  We will just bump the
			 timeout value.  Note that the new record has been
			 allocated on the stack and need not be freed.  */
		      dh->timeout = dataset->head.timeout;
		      ++dh->nreloads;
		    }
		  else
		    {
		      /* We have to create a new record.  Just allocate
			 appropriate memory and copy it.  */
		      struct dataset *newp
			= (struct dataset *) mempool_alloc (db,
							    total
							    + req->key_len);
		      if (__builtin_expect (newp != NULL, 1))
			{
			  /* Adjust pointer into the memory block.  */
			  key_copy = (char *) newp + (key_copy
						      - (char *) dataset);

			  dataset = memcpy (newp, dataset,
					    total + req->key_len);
			  alloca_used = false;
			}
		      else
			++db->head->addfailed;

		      /* Mark the old record as obsolete.  */
		      dh->usable = false;
		    }
		}
	      else
		{
		  /* We write the dataset before inserting it to the
		     database since while inserting this thread might
		     block and so would unnecessarily let the receiver
		     wait.  */
		  assert (fd != -1);

#ifdef HAVE_SENDFILE
		  if (__builtin_expect (db->mmap_used, 1) && !alloca_used)
		    {
		      assert (db->wr_fd != -1);
		      assert ((char *) &dataset->resp > (char *) db->data);
		      assert ((char *) &dataset->resp - (char *) db->head
			      + total
			      <= (sizeof (struct database_pers_head)
				  + db->head->module * sizeof (ref_t)
				  + db->head->data_size));
		      ssize_t written;
		      written = sendfileall (fd, db->wr_fd,
					     (char *) &dataset->resp
					     - (char *) db->head, total);
# ifndef __ASSUME_SENDFILE
		      if (written == -1 && errno == ENOSYS)
			goto use_write;
# endif
		    }
		  else
# ifndef __ASSUME_SENDFILE
		  use_write:
# endif
#endif
		    writeall (fd, &dataset->resp, total);
		}

	      goto out;
	    }

	}

      if (nss_next_action (nip, status[1]) == NSS_ACTION_RETURN)
	break;

      if (nip->next == NULL)
	no_more = -1;
      else
	nip = nip->next;
    }

  /* No result found.  Create a negative result record.  */
  if (he != NULL && rc4 == EAGAIN)
    {
      /* If we have an old record available but cannot find one now
	 because the service is not available we keep the old record
	 and make sure it does not get removed.  */
      if (reload_count != UINT_MAX && dh->nreloads == reload_count)
	/* Do not reset the value if we never not reload the record.  */
	dh->nreloads = reload_count - 1;
    }
  else
    {
      /* We have no data.  This means we send the standard reply for
	 this case.  */
      total = sizeof (notfound);

      if (fd != -1)
	TEMP_FAILURE_RETRY (send (fd, &notfound, total, MSG_NOSIGNAL));

      dataset = mempool_alloc (db, sizeof (struct dataset) + req->key_len);
      /* If we cannot permanently store the result, so be it.  */
      if (dataset != NULL)
	{
	  dataset->head.allocsize = sizeof (struct dataset) + req->key_len;
	  dataset->head.recsize = total;
	  dataset->head.notfound = true;
	  dataset->head.nreloads = 0;
	  dataset->head.usable = true;

	  /* Compute the timeout time.  */
	  dataset->head.timeout = time (NULL) + db->negtimeout;

	  /* This is the reply.  */
	  memcpy (&dataset->resp, &notfound, total);

	  /* Copy the key data.  */
	  key_copy = memcpy (dataset->strdata, key, req->key_len);
	}
      else
	++db->head->addfailed;
   }

 out:
  _res.options = old_res_options;

  if (dataset != NULL && !alloca_used)
    {
      /* If necessary, we also propagate the data to disk.  */
      if (db->persistent)
	{
	  // XXX async OK?
	  uintptr_t pval = (uintptr_t) dataset & ~pagesize_m1;
	  msync ((void *) pval,
		 ((uintptr_t) dataset & pagesize_m1) + total + req->key_len,
		 MS_ASYNC);
	}

      /* Now get the lock to safely insert the records.  */
      pthread_rwlock_rdlock (&db->lock);

      if (cache_add (req->type, key_copy, req->key_len, &dataset->head, true,
		     db, uid) < 0)
	/* Ensure the data can be recovered.  */
	dataset->head.usable = false;

      pthread_rwlock_unlock (&db->lock);

      /* Mark the old entry as obsolete.  */
      if (dh != NULL)
	dh->usable = false;
    }
}


void
addhstai (struct database_dyn *db, int fd, request_header *req, void *key,
	  uid_t uid)
{
  addhstaiX (db, fd, req, key, uid, NULL, NULL);
}


void
readdhstai (struct database_dyn *db, struct hashentry *he, struct datahead *dh)
{
  request_header req =
    {
      .type = GETAI,
      .key_len = he->len
    };

  addhstaiX (db, -1, &req, db->data + he->key, he->owner, he, dh);
}
