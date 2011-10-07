/* Cache handling for netgroup lookup.
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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

#include <alloca.h>
#include <assert.h>
#include <errno.h>
#include <libintl.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../inet/netgroup.h"
#include "nscd.h"
#include "dbg_log.h"
#ifdef HAVE_SENDFILE
# include <kernel-features.h>
#endif


/* This is the standard reply in case the service is disabled.  */
static const netgroup_response_header disabled =
{
  .version = NSCD_VERSION,
  .found = -1,
  .nresults = 0,
  .result_len = 0
};

/* This is the struct describing how to write this record.  */
const struct iovec netgroup_iov_disabled =
{
  .iov_base = (void *) &disabled,
  .iov_len = sizeof (disabled)
};


/* This is the standard reply in case we haven't found the dataset.  */
static const netgroup_response_header notfound =
{
  .version = NSCD_VERSION,
  .found = 0,
  .nresults = 0,
  .result_len = 0
};


struct dataset
{
  struct datahead head;
  netgroup_response_header resp;
  char strdata[0];
};


static time_t
addgetnetgrentX (struct database_dyn *db, int fd, request_header *req,
		 const char *key, uid_t uid, struct hashentry *he,
		 struct datahead *dh, struct dataset **resultp)
{
  if (__builtin_expect (debug_level > 0, 0))
    {
      if (he == NULL)
	dbg_log (_("Haven't found \"%s\" in netgroup cache!"), key);
      else
	dbg_log (_("Reloading \"%s\" in netgroup cache!"), key);
    }

  static service_user *netgroup_database;
  time_t timeout;
  struct dataset *dataset;
  bool cacheable = false;
  ssize_t total;

  char *key_copy = NULL;
  struct __netgrent data;
  size_t buflen = MAX (1024, sizeof (*dataset) + req->key_len);
  size_t buffilled = sizeof (*dataset);
  char *buffer = NULL;
  size_t nentries = 0;
  bool use_malloc = false;
  size_t group_len = strlen (key) + 1;
  union
  {
    struct name_list elem;
    char mem[sizeof (struct name_list) + group_len];
  } first_needed;

  if (netgroup_database == NULL
      && __nss_database_lookup ("netgroup", NULL, NULL, &netgroup_database))
    {
      /* No such service.  */
      total = sizeof (notfound);
      timeout = time (NULL) + db->negtimeout;

      if (fd != -1)
	TEMP_FAILURE_RETRY (send (fd, &notfound, total, MSG_NOSIGNAL));

      dataset = mempool_alloc (db, sizeof (struct dataset) + req->key_len, 1);
      /* If we cannot permanently store the result, so be it.  */
      if (dataset != NULL)
	{
	  dataset->head.allocsize = sizeof (struct dataset) + req->key_len;
	  dataset->head.recsize = total;
	  dataset->head.notfound = true;
	  dataset->head.nreloads = 0;
	  dataset->head.usable = true;

	  /* Compute the timeout time.  */
	  timeout = dataset->head.timeout = time (NULL) + db->negtimeout;
	  dataset->head.ttl = db->negtimeout;

	  /* This is the reply.  */
	  memcpy (&dataset->resp, &notfound, total);

	  /* Copy the key data.  */
	  memcpy (dataset->strdata, key, req->key_len);

	  cacheable = true;
	}

      goto writeout;
    }

  memset (&data, '\0', sizeof (data));
  buffer = alloca (buflen);
  first_needed.elem.next = &first_needed.elem;
  memcpy (first_needed.elem.name, key, group_len);
  data.needed_groups = &first_needed.elem;

  while (data.needed_groups != NULL)
    {
      /* Add the next group to the list of those which are known.  */
      struct name_list *this_group = data.needed_groups->next;
      if (this_group == data.needed_groups)
	data.needed_groups = NULL;
      else
	data.needed_groups->next = this_group->next;
      this_group->next = data.known_groups;
      data.known_groups = this_group;

      union
      {
	enum nss_status (*f) (const char *, struct __netgrent *);
	void *ptr;
      } setfct;

      service_user *nip = netgroup_database;
      int no_more = __nss_lookup (&nip, "setnetgrent", NULL, &setfct.ptr);
      while (!no_more)
	{
	  enum nss_status status
	    = DL_CALL_FCT (*setfct.f, (data.known_groups->name, &data));

	  if (status == NSS_STATUS_SUCCESS)
	    {
	      union
	      {
		enum nss_status (*f) (struct __netgrent *, char *, size_t,
				      int *);
		void *ptr;
	      } getfct;
	      getfct.ptr = __nss_lookup_function (nip, "getnetgrent_r");
	      if (getfct.f != NULL)
		while (1)
		  {
		    int e;
		    status = getfct.f (&data, buffer + buffilled,
				       buflen - buffilled, &e);
		    if (status == NSS_STATUS_RETURN)
		      /* This was the last one for this group.  Look
			 at next group if available.  */
		      break;
		    if (status == NSS_STATUS_SUCCESS)
		      {
			if (data.type == triple_val)
			  {
			    const char *nhost = data.val.triple.host;
			    const char *nuser = data.val.triple.user;
			    const char *ndomain = data.val.triple.domain;

			    if (data.val.triple.host > data.val.triple.user
				|| data.val.triple.user > data.val.triple.domain)
			      {
				const char *last = MAX (nhost,
							MAX (nuser, ndomain));
				size_t bufused = (last + strlen (last) + 1
						  - buffer);

				/* We have to make temporary copies.  */
				size_t hostlen = strlen (nhost) + 1;
				size_t userlen = strlen (nuser) + 1;
				size_t domainlen = strlen (ndomain) + 1;
				size_t needed = hostlen + userlen + domainlen;

				if (buflen - req->key_len - bufused < needed)
				  {
				    size_t newsize = MAX (2 * buflen,
							  buflen + 2 * needed);
				    if (use_malloc || newsize > 1024 * 1024)
				      {
					buflen = newsize;
					char *newbuf = xrealloc (use_malloc
								 ? buffer
								 : NULL,
								 buflen);

					buffer = newbuf;
					use_malloc = true;
				      }
				    else
				      extend_alloca (buffer, buflen, newsize);
				  }

				nhost = memcpy (buffer + bufused,
						nhost, hostlen);
				nuser = memcpy ((char *) nhost + hostlen,
						nuser, userlen);
				ndomain = memcpy ((char *) nuser + userlen,
						  ndomain, domainlen);
			      }

			    char *wp = buffer + buffilled;
			    wp = stpcpy (wp, nhost) + 1;
			    wp = stpcpy (wp, nuser) + 1;
			    wp = stpcpy (wp, ndomain) + 1;
			    buffilled = wp - buffer;
			    ++nentries;
			  }
			else
			  {
			    /* Check that the group has not been
			       requested before.  */
			    struct name_list *runp = data.needed_groups;
			    if (runp != NULL)
			      while (1)
				{
				  if (strcmp (runp->name, data.val.group) == 0)
				    break;

				  runp = runp->next;
				  if (runp == data.needed_groups)
				    {
				      runp = NULL;
				      break;
				    }
				}

			    if (runp == NULL)
			      {
				runp = data.known_groups;
				while (runp != NULL)
				  if (strcmp (runp->name, data.val.group) == 0)
				    break;
				  else
				    runp = runp->next;
				}

			    if (runp == NULL)
			      {
				/* A new group is requested.  */
				size_t namelen = strlen (data.val.group) + 1;
				struct name_list *newg = alloca (sizeof (*newg)
								 + namelen);
				memcpy (newg->name, data.val.group, namelen);
				if (data.needed_groups == NULL)
				  data.needed_groups = newg->next = newg;
				else
				  {
				    newg->next = data.needed_groups->next;
				    data.needed_groups->next = newg;
				    data.needed_groups = newg;
				  }
			      }
			  }
		      }
		    else if (status == NSS_STATUS_UNAVAIL && e == ERANGE)
		      {
			size_t newsize = 2 * buflen;
			if (use_malloc || newsize > 1024 * 1024)
			  {
			    buflen = newsize;
			    char *newbuf = xrealloc (use_malloc
						     ? buffer : NULL, buflen);

			    buffer = newbuf;
			    use_malloc = true;
			  }
			else
			  extend_alloca (buffer, buflen, newsize);
		      }
		  }

	      enum nss_status (*endfct) (struct __netgrent *);
	      endfct = __nss_lookup_function (nip, "endnetgrent");
	      if (endfct != NULL)
		(void) DL_CALL_FCT (*endfct, (&data));

	      break;
	    }

	  no_more = __nss_next2 (&nip, "setnetgrent", NULL, &setfct.ptr,
				 status, 0);
	}
    }

  total = buffilled;

  /* Fill in the dataset.  */
  dataset = (struct dataset *) buffer;
  dataset->head.allocsize = total + req->key_len;
  dataset->head.recsize = total - offsetof (struct dataset, resp);
  dataset->head.notfound = false;
  dataset->head.nreloads = he == NULL ? 0 : (dh->nreloads + 1);
  dataset->head.usable = true;
  dataset->head.ttl = db->postimeout;
  timeout = dataset->head.timeout = time (NULL) + dataset->head.ttl;

  dataset->resp.version = NSCD_VERSION;
  dataset->resp.found = 1;
  dataset->resp.nresults = nentries;
  dataset->resp.result_len = buffilled - sizeof (*dataset);

  assert (buflen - buffilled >= req->key_len);
  key_copy = memcpy (buffer + buffilled, key, req->key_len);
  buffilled += req->key_len;

  /* Now we can determine whether on refill we have to create a new
     record or not.  */
  if (he != NULL)
    {
      assert (fd == -1);

      if (dataset->head.allocsize == dh->allocsize
	  && dataset->head.recsize == dh->recsize
	  && memcmp (&dataset->resp, dh->data,
		     dh->allocsize - offsetof (struct dataset, resp)) == 0)
	{
	  /* The data has not changed.  We will just bump the timeout
	     value.  Note that the new record has been allocated on
	     the stack and need not be freed.  */
	  dh->timeout = dataset->head.timeout;
	  dh->ttl = dataset->head.ttl;
	  ++dh->nreloads;
	  dataset = (struct dataset *) dh;

	  goto out;
	}
    }

  {
    struct dataset *newp
      = (struct dataset *) mempool_alloc (db, total + req->key_len, 1);
    if (__builtin_expect (newp != NULL, 1))
      {
	/* Adjust pointer into the memory block.  */
	key_copy = (char *) newp + (key_copy - buffer);

	dataset = memcpy (newp, dataset, total + req->key_len);
	cacheable = true;

	if (he != NULL)
	  /* Mark the old record as obsolete.  */
	  dh->usable = false;
      }
  }

  if (he == NULL && fd != -1)
    {
      /* We write the dataset before inserting it to the database
	 since while inserting this thread might block and so would
	 unnecessarily let the receiver wait.  */
    writeout:
#ifdef HAVE_SENDFILE
      if (__builtin_expect (db->mmap_used, 1) && cacheable)
	{
	  assert (db->wr_fd != -1);
	  assert ((char *) &dataset->resp > (char *) db->data);
	  assert ((char *) dataset - (char *) db->head + total
		  <= (sizeof (struct database_pers_head)
		      + db->head->module * sizeof (ref_t)
		      + db->head->data_size));
# ifndef __ASSUME_SENDFILE
	  ssize_t written =
# endif
	    sendfileall (fd, db->wr_fd, (char *) &dataset->resp
			 - (char *) db->head, dataset->head.recsize);
# ifndef __ASSUME_SENDFILE
	  if (written == -1 && errno == ENOSYS)
	    goto use_write;
# endif
	}
      else
	{
# ifndef __ASSUME_SENDFILE
	use_write:
# endif
#endif
	  writeall (fd, &dataset->resp, dataset->head.recsize);
	}
    }

  if (cacheable)
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

      (void) cache_add (req->type, key_copy, req->key_len, &dataset->head,
			true, db, uid, he == NULL);

      pthread_rwlock_unlock (&db->lock);

      /* Mark the old entry as obsolete.  */
      if (dh != NULL)
	dh->usable = false;
    }

 out:
  if (use_malloc)
    free (buffer);

  *resultp = dataset;

  return timeout;
}


static time_t
addinnetgrX (struct database_dyn *db, int fd, request_header *req,
	     char *key, uid_t uid, struct hashentry *he,
	     struct datahead *dh)
{
  const char *group = key;
  key = (char *) rawmemchr (key, '\0') + 1;
  size_t group_len = key - group - 1;
  const char *host = *key++ ? key : NULL;
  if (host != NULL)
    key = (char *) rawmemchr (key, '\0') + 1;
  const char *user = *key++ ? key : NULL;
  if (user != NULL)
    key = (char *) rawmemchr (key, '\0') + 1;
  const char *domain = *key++ ? key : NULL;

  if (__builtin_expect (debug_level > 0, 0))
    {
      if (he == NULL)
	dbg_log (_("Haven't found \"%s (%s,%s,%s)\" in netgroup cache!"),
		 group, host ?: "", user ?: "", domain ?: "");
      else
	dbg_log (_("Reloading \"%s (%s,%s,%s)\" in netgroup cache!"),
		 group, host ?: "", user ?: "", domain ?: "");
    }

  struct dataset *result = (struct dataset *) cache_search (GETNETGRENT,
							    group, group_len,
							    db, uid);
  time_t timeout;
  if (result != NULL)
    timeout = result->head.timeout;
  else
    {
      request_header req_get =
	{
	  .type = GETNETGRENT,
	  .key_len = group_len
	};
      timeout = addgetnetgrentX (db, -1, &req_get, group, uid, NULL, NULL,
				 &result);
    }

  struct indataset
  {
    struct datahead head;
    innetgroup_response_header resp;
  } *dataset
      = (struct indataset *) mempool_alloc (db,
					    sizeof (*dataset) + req->key_len,
					    1);
  struct indataset dataset_mem;
  bool cacheable = true;
  if (__builtin_expect (dataset == NULL, 0))
    {
      cacheable = false;
      dataset = &dataset_mem;
    }

  dataset->head.allocsize = sizeof (*dataset) + req->key_len;
  dataset->head.recsize = sizeof (innetgroup_response_header);
  dataset->head.notfound = result->head.notfound;
  dataset->head.nreloads = he == NULL ? 0 : (dh->nreloads + 1);
  dataset->head.usable = true;
  dataset->head.ttl = result->head.ttl;
  dataset->head.timeout = timeout;

  dataset->resp.version = NSCD_VERSION;
  dataset->resp.found = result->resp.found;
  /* Until we find a matching entry the result is 0.  */
  dataset->resp.result = 0;

  char *key_copy = memcpy ((char *) (dataset + 1), group, req->key_len);

  if (dataset->resp.found)
    {
      const char *triplets = (const char *) (&result->resp + 1);

      for (nscd_ssize_t i = result->resp.nresults; i > 0; --i)
	{
	  bool success = true;

	  if (host != NULL)
	    success = strcmp (host, triplets) == 0;
	  triplets = (const char *) rawmemchr (triplets, '\0') + 1;

	  if (success && user != NULL)
	    success = strcmp (user, triplets) == 0;
	  triplets = (const char *) rawmemchr (triplets, '\0') + 1;

	  if (success && (domain == NULL || strcmp (domain, triplets) == 0))
	    {
	      dataset->resp.result = 1;
	      break;
	    }
	  triplets = (const char *) rawmemchr (triplets, '\0') + 1;
	}
    }

  if (he != NULL && dh->data[0].innetgroupdata.result == dataset->resp.result)
    {
      /* The data has not changed.  We will just bump the timeout
	 value.  Note that the new record has been allocated on
	 the stack and need not be freed.  */
      dh->timeout = timeout;
      dh->ttl = dataset->head.ttl;
      ++dh->nreloads;
      return timeout;
    }

  if (he == NULL)
    {
      /* We write the dataset before inserting it to the database
	 since while inserting this thread might block and so would
	 unnecessarily let the receiver wait.  */
       assert (fd != -1);

#ifdef HAVE_SENDFILE
      if (__builtin_expect (db->mmap_used, 1) && cacheable)
	{
	  assert (db->wr_fd != -1);
	  assert ((char *) &dataset->resp > (char *) db->data);
	  assert ((char *) dataset - (char *) db->head + sizeof (*dataset)
		  <= (sizeof (struct database_pers_head)
		      + db->head->module * sizeof (ref_t)
		      + db->head->data_size));
# ifndef __ASSUME_SENDFILE
	  ssize_t written =
# endif
	    sendfileall (fd, db->wr_fd,
			 (char *) &dataset->resp - (char *) db->head,
			 sizeof (innetgroup_response_header));
# ifndef __ASSUME_SENDFILE
	  if (written == -1 && errno == ENOSYS)
	    goto use_write;
# endif
	}
      else
	{
# ifndef __ASSUME_SENDFILE
	use_write:
# endif
#endif
	  writeall (fd, &dataset->resp, sizeof (innetgroup_response_header));
	}
    }

  if (cacheable)
    {
      /* If necessary, we also propagate the data to disk.  */
      if (db->persistent)
	{
	  // XXX async OK?
	  uintptr_t pval = (uintptr_t) dataset & ~pagesize_m1;
	  msync ((void *) pval,
		 ((uintptr_t) dataset & pagesize_m1) + sizeof (*dataset)
		 + req->key_len,
		 MS_ASYNC);
	}

      (void) cache_add (req->type, key_copy, req->key_len, &dataset->head,
			true, db, uid, he == NULL);

      pthread_rwlock_unlock (&db->lock);

      /* Mark the old entry as obsolete.  */
      if (dh != NULL)
	dh->usable = false;
    }

  return timeout;
}


void
addgetnetgrent (struct database_dyn *db, int fd, request_header *req,
		void *key, uid_t uid)
{
  struct dataset *ignore;

  addgetnetgrentX (db, fd, req, key, uid, NULL, NULL, &ignore);
}


time_t
readdgetnetgrent (struct database_dyn *db, struct hashentry *he,
		  struct datahead *dh)
{
  request_header req =
    {
      .type = GETNETGRENT,
      .key_len = he->len
    };
  struct dataset *ignore;

  return addgetnetgrentX (db, -1, &req, db->data + he->key, he->owner, he, dh,
			  &ignore);
}


void
addinnetgr (struct database_dyn *db, int fd, request_header *req,
	    void *key, uid_t uid)
{
  addinnetgrX (db, fd, req, key, uid, NULL, NULL);
}


time_t
readdinnetgr (struct database_dyn *db, struct hashentry *he,
	      struct datahead *dh)
{
  request_header req =
    {
      .type = INNETGR,
      .key_len = he->len
    };

  return addinnetgrX (db, -1, &req, db->data + he->key, he->owner, he, dh);
}
