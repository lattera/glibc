/* Cache handling for host lookup.
   Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libintl.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include "nscd.h"
#include "dbg_log.h"


/* This is the standard reply in case the service is disabled.  */
static const hst_response_header disabled =
{
  version: NSCD_VERSION,
  found: -1,
  h_name_len: 0,
  h_aliases_cnt: 0,
  h_addrtype: -1,
  h_length: -1,
  h_addr_list_cnt: 0,
  error: NETDB_INTERNAL
};

/* This is the struct describing how to write this record.  */
const struct iovec hst_iov_disabled =
{
  iov_base: (void *) &disabled,
  iov_len: sizeof (disabled)
};


/* This is the standard reply in case we haven't found the dataset.  */
static const hst_response_header notfound =
{
  version: NSCD_VERSION,
  found: 0,
  h_name_len: 0,
  h_aliases_cnt: 0,
  h_addrtype: -1,
  h_length: -1,
  h_addr_list_cnt: 0,
  error: HOST_NOT_FOUND
};

/* This is the struct describing how to write this record.  */
static const struct iovec iov_notfound =
{
  iov_base: (void *) &notfound,
  iov_len: sizeof (notfound)
};


struct hostdata
{
  hst_response_header resp;
  char strdata[0];
};


static void
cache_addhst (struct database *db, int fd, request_header *req, void *key,
	      struct hostent *hst, uid_t owner)
{
  ssize_t total;
  ssize_t written;
  time_t t = time (NULL);

  if (hst == NULL)
    {
      /* We have no data.  This means we send the standard reply for this
	 case.  */
      void *copy;

      total = sizeof (notfound);

      written = writev (fd, &iov_notfound, 1);

      copy = malloc (req->key_len);
      if (copy == NULL)
	error (EXIT_FAILURE, errno, _("while allocating key copy"));
      memcpy (copy, key, req->key_len);

      /* Compute the timeout time.  */
      t += db->negtimeout;

      /* Now get the lock to safely insert the records.  */
      pthread_rwlock_rdlock (&db->lock);

      cache_add (req->type, copy, req->key_len, &notfound,
		 sizeof (notfound), (void *) -1, 0, t, db, owner);

      pthread_rwlock_unlock (&db->lock);
    }
  else
    {
      /* Determine the I/O structure.  */
      struct hostdata *data;
      size_t h_name_len = strlen (hst->h_name) + 1;
      size_t h_aliases_cnt;
      uint32_t *h_aliases_len;
      size_t h_addr_list_cnt;
      int addr_list_type;
      char *addresses;
      char *aliases;
      char *key_copy = NULL;
      char *cp;
      size_t cnt;

      /* Determine the number of aliases.  */
      h_aliases_cnt = 0;
      for (cnt = 0; hst->h_aliases[cnt] != NULL; ++cnt)
	++h_aliases_cnt;
      /* Determine the length of all aliases.  */
      h_aliases_len = (uint32_t *) alloca (h_aliases_cnt * sizeof (uint32_t));
      total = 0;
      for (cnt = 0; cnt < h_aliases_cnt; ++cnt)
	{
	  h_aliases_len[cnt] = strlen (hst->h_aliases[cnt]) + 1;
	  total += h_aliases_len[cnt];
	}

      /* Determine the number of addresses.  */
      h_addr_list_cnt = 0;
      for (cnt = 0; hst->h_addr_list[cnt]; ++cnt)
	++h_addr_list_cnt;

      /* We allocate all data in one memory block: the iov vector,
	 the response header and the dataset itself.  */
      total += (sizeof (struct hostdata)
		+ h_name_len
		+ h_aliases_cnt * sizeof (uint32_t)
		+ h_addr_list_cnt * hst->h_length);

      data = (struct hostdata *) malloc (total + req->key_len);
      if (data == NULL)
	/* There is no reason to go on.  */
	error (EXIT_FAILURE, errno, _("while allocating cache entry"));

      data->resp.found = 1;
      data->resp.h_name_len = h_name_len;
      data->resp.h_aliases_cnt = h_aliases_cnt;
      data->resp.h_addrtype = hst->h_addrtype;
      data->resp.h_length = hst->h_length;
      data->resp.h_addr_list_cnt = h_addr_list_cnt;
      data->resp.error = NETDB_SUCCESS;

      cp = data->strdata;

      cp = mempcpy (cp, hst->h_name, h_name_len);
      cp = mempcpy (cp, h_aliases_len, h_aliases_cnt * sizeof (uint32_t));

      /* The normal addresses first.  */
      addresses = cp;
      for (cnt = 0; cnt < h_addr_list_cnt; ++cnt)
	cp = mempcpy (cp, hst->h_addr_list[cnt], hst->h_length);

      /* Then the aliases.  */
      aliases = cp;
      for (cnt = 0; cnt < h_aliases_cnt; ++cnt)
	cp = mempcpy (cp, hst->h_aliases[cnt], h_aliases_len[cnt]);

      assert (cp == data->strdata + total - sizeof (hst_response_header));

      /* If we are adding a GETHOSTBYNAME{,v6} entry we must be prepared
	 that the answer we get from the NSS does not contain the key
	 itself.  This is the case if the resolver is used and the name
	 is extended by the domainnames from /etc/resolv.conf.  Therefore
	 we explicitly add the name here.  */
      if (req->type == GETHOSTBYNAME || req->type == GETHOSTBYNAMEv6)
	key_copy = memcpy (cp, key, req->key_len);

      /* We write the dataset before inserting it to the database
	 since while inserting this thread might block and so would
	 unnecessarily let the receiver wait.  */
      written = write (fd, data, total);

      addr_list_type = (hst->h_length == NS_INADDRSZ
			? GETHOSTBYADDR : GETHOSTBYADDRv6);

      /* Compute the timeout time.  */
      t += db->postimeout;

      /* Now get the lock to safely insert the records.  */
      pthread_rwlock_rdlock (&db->lock);

      /* First add all the aliases.  If the record contains more than
         one IP address (used for load balancing etc) don't cache the
         entry.  This is something the current cache handling cannot
         handle and it is more than questionable whether it is
         worthwhile complicating the cache handling just for handling
         such a special case.  */
      if (hst->h_addr_list[1] == NULL)
	for (cnt = 0; cnt < h_aliases_cnt; ++cnt)
	  {
	    if (addr_list_type == GETHOSTBYADDR)
	      cache_add (GETHOSTBYNAME, aliases, h_aliases_len[cnt], data,
			 total, data, 0, t, db, owner);

	    cache_add (GETHOSTBYNAMEv6, aliases, h_aliases_len[cnt], data,
		       total, data, 0, t, db, owner);

	    aliases += h_aliases_len[cnt];
	  }

      /* Next the normal addresses.  */
      for (cnt = 0; cnt < h_addr_list_cnt; ++cnt)
	{
	  cache_add (addr_list_type, addresses, hst->h_length, data, total,
		     data, 0, t, db, owner);
	  addresses += hst->h_length;
	}

      /* If necessary the IPv6 addresses.  */
      if (addr_list_type == GETHOSTBYADDR)
	for (cnt = 0; cnt < h_addr_list_cnt; ++cnt)
	  {
	    cache_add (GETHOSTBYADDRv6, addresses, IN6ADDRSZ, data, total,
		       data, 0, t, db, owner);
	    addresses += IN6ADDRSZ;
	  }

      /* Avoid adding names if more than one address is available.  See
	 above for more info.  */
      if (hst->h_addr_list[1] == NULL)
	{
	  /* If necessary add the key for this request.  */
	  if (req->type == GETHOSTBYNAME || req->type == GETHOSTBYNAMEv6)
	    {
	      if (addr_list_type == GETHOSTBYADDR)
		cache_add (GETHOSTBYNAME, key_copy, req->key_len, data, total,
			   data, 0, t, db, owner);
	      cache_add (GETHOSTBYNAMEv6, key_copy, req->key_len, data,
			 total, data, 0, t, db, owner);
	    }

	  /* And finally the name.  We mark this as the last entry.  */
	  if (addr_list_type == GETHOSTBYADDR)
	    cache_add (GETHOSTBYNAME, data->strdata, h_name_len, data, total,
		       data, 0, t, db, owner);
	  cache_add (GETHOSTBYNAMEv6, data->strdata, h_name_len, data,
		     total, data, 1, t, db, owner);
	}

      pthread_rwlock_unlock (&db->lock);
    }

  if (written != total && debug_level > 0)
    {
      char buf[256];
      dbg_log (_("short write in %s: %s"),  __FUNCTION__,
	       strerror_r (errno, buf, sizeof (buf)));
    }
}


void
addhstbyname (struct database *db, int fd, request_header *req,
	      void *key, uid_t uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 512;
  char *buffer = alloca (buflen);
  struct hostent resultbuf;
  struct hostent *hst;
  uid_t oldeuid = 0;

  if (debug_level > 0)
    dbg_log (_("Haven't found \"%s\" in hosts cache!"), (char *)key);

  if (secure[hstdb])
    {
      oldeuid = geteuid ();
      seteuid (uid);
    }

  while (__gethostbyname2_r (key, AF_INET, &resultbuf, buffer, buflen,
  			     &hst, &h_errno) != 0
	 && h_errno == NETDB_INTERNAL
	 && errno == ERANGE)
    {
      errno = 0;
      buflen += 256;
      buffer = alloca (buflen);
    }

  if (secure[hstdb])
    seteuid (oldeuid);

  cache_addhst (db, fd, req, key, hst, uid);
}


void
addhstbyaddr (struct database *db, int fd, request_header *req,
	      void *key, uid_t uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 512;
  char *buffer = alloca (buflen);
  struct hostent resultbuf;
  struct hostent *hst;
  uid_t oldeuid = 0;

  if (debug_level > 0)
    {
      char buf[INET_ADDRSTRLEN];
      dbg_log (_("Haven't found \"%s\" in hosts cache!"),
	       inet_ntop (AF_INET, key, buf, sizeof (buf)));
    }

  if (secure[hstdb])
    {
      oldeuid = geteuid ();
      seteuid (uid);
    }

  while (__gethostbyaddr_r (key, NS_INADDRSZ, AF_INET, &resultbuf, buffer,
  			    buflen, &hst, &h_errno) != 0
	 && h_errno == NETDB_INTERNAL
	 && errno == ERANGE)
    {
      errno = 0;
      buflen += 256;
      buffer = alloca (buflen);
    }

  if (secure[hstdb])
    seteuid (oldeuid);

  cache_addhst (db, fd, req, key, hst, uid);
}


void
addhstbynamev6 (struct database *db, int fd, request_header *req,
		void *key, uid_t uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 512;
  char *buffer = alloca (buflen);
  struct hostent resultbuf;
  struct hostent *hst;
  uid_t oldeuid = 0;

  if (debug_level > 0)
    {
      char buf[INET6_ADDRSTRLEN];

      dbg_log (_("Haven't found \"%s\" in hosts cache!"),
	       inet_ntop (AF_INET6, key, buf, sizeof (buf)));
    }

  if (secure[hstdb])
    {
      oldeuid = geteuid ();
      seteuid (uid);
    }

  while (__gethostbyname2_r (key, AF_INET6, &resultbuf, buffer, buflen,
  			     &hst, &h_errno) != 0
	 && h_errno == NETDB_INTERNAL
	 && errno == ERANGE)
    {
      errno = 0;
      buflen += 256;
      buffer = alloca (buflen);
    }

  if (secure[hstdb])
    seteuid (oldeuid);

  cache_addhst (db, fd, req, key, hst, uid);
}


void
addhstbyaddrv6 (struct database *db, int fd, request_header *req,
		void *key, uid_t uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 512;
  char *buffer = alloca (buflen);
  struct hostent resultbuf;
  struct hostent *hst;
  uid_t oldeuid = 0;

  if (debug_level > 0)
    {
      char buf[INET6_ADDRSTRLEN];
      dbg_log (_("Haven't found \"%s\" in hosts cache!"),
	       inet_ntop (AF_INET6, key, buf, sizeof (buf)));
    }

  if (secure[hstdb])
    {
      oldeuid = geteuid ();
      seteuid (uid);
    }

  while (__gethostbyaddr_r (key, NS_IN6ADDRSZ, AF_INET6, &resultbuf,
  			    buffer, buflen, &hst, &h_errno) != 0
	 && h_errno == NETDB_INTERNAL
	 && errno == ERANGE)
    {
      errno = 0;
      buflen += 256;
      buffer = alloca (buflen);
    }

  if (secure[hstdb])
    seteuid (oldeuid);

  cache_addhst (db, fd, req, key, hst, uid);
}
