/* Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2004.

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
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <not-cancel.h>

#include "nscd-client.h"
#include "nscd_proto.h"


/* Define in nscd_gethst_r.c.  */
extern int __nss_not_use_nscd_hosts;


libc_locked_map_ptr (map_handle);
/* Note that we only free the structure if necessary.  The memory
   mapping is not removed since it is not visible to the malloc
   handling.  */
libc_freeres_fn (ai_map_free)
{

  if (map_handle.mapped != NO_MAPPING)
    free (map_handle.mapped);
}


int
__nscd_getai (const char *key, struct nscd_ai_result **result, int *h_errnop)
{
  size_t keylen = strlen (key) + 1;
  const ai_response_header *ai_resp = NULL;
  struct nscd_ai_result *resultbuf = NULL;
  const char *recend = (const char *) ~UINTMAX_C (0);
  char *respdata = NULL;
  int retval = -1;
  int sock = -1;
  int gc_cycle;

  /* If the mapping is available, try to search there instead of
     communicating with the nscd.  */
  struct mapped_database *mapped = __nscd_get_map_ref (GETFDHST, "hosts",
						       &map_handle, &gc_cycle);
 retry:
  if (mapped != NO_MAPPING)
    {
      const struct datahead *found = __nscd_cache_search (GETAI, key, keylen,
							  mapped);
      if (found != NULL)
	{
	  ai_resp = &found->data[0].aidata;
	  respdata = (char *) (ai_resp + 1);
	  recend = (const char *) found->data + found->recsize;
	}
    }

  /* If we do not have the cache mapped, try to get the data over the
     socket.  */
  ai_response_header ai_resp_mem;
  if (ai_resp == NULL)
    {
      sock = __nscd_open_socket (key, keylen, GETAI, &ai_resp_mem,
				 sizeof (ai_resp_mem));
      if (sock == -1)
	{
	  /* nscd not running or wrong version or hosts caching disabled.  */
	  __nss_not_use_nscd_hosts = 1;
	  goto out;
	}

      ai_resp = &ai_resp_mem;
    }

  if (ai_resp->found == 1)
    {
      size_t datalen = ai_resp->naddrs + ai_resp->addrslen + ai_resp->canonlen;

      /* This check is really only affects the case where the data
	 comes from the mapped cache.  */
      if ((char *) (ai_resp + 1) + datalen > recend)
	{
	  assert (sock == -1);
	  goto out;
	}

      /* Create result.  */
      resultbuf = (struct nscd_ai_result *) malloc (sizeof (*resultbuf)
						    + datalen);
      if (resultbuf == NULL)
	{
	  *h_errnop = NETDB_INTERNAL;
	  return -1;
	}

      /* Set up the data structure, including pointers.  */
      resultbuf->naddrs = ai_resp->naddrs;
      resultbuf->addrs = (char *) (resultbuf + 1);
      resultbuf->family = (uint8_t *) (resultbuf->addrs + ai_resp->addrslen);
      if (ai_resp->canonlen != 0)
	resultbuf->canon = (char *) (resultbuf->family + resultbuf->naddrs);
      else
	resultbuf->canon = NULL;

      if (respdata == NULL)
	{
	  /* Read the data from the socket.  */
	  if ((size_t) TEMP_FAILURE_RETRY (__read (sock, resultbuf + 1,
						   datalen)) == datalen)
	    {
	      retval = 0;
	      *result = resultbuf;
	    }
	}
      else
	{
	  /* Copy the data in the block.  */
	  memcpy (resultbuf + 1, respdata, datalen);

	  retval = 0;
	  *result = resultbuf;
	}
    }
  else
    {
      /* Store the error number.  */
      *h_errnop = ai_resp->error;

      /* The `errno' to some value != ERANGE.  */
      __set_errno (ENOENT);
      /* Even though we have not found anything, the result is zero.  */
      retval = 0;
    }

  if (sock != -1)
    close_not_cancel_no_status (sock);
 out:
  if (__nscd_drop_map_ref (mapped, gc_cycle) != 0)
    /* When we come here this means there has been a GC cycle while we
       were looking for the data.  This means the data might have been
       inconsistent.  Retry.  */
    goto retry;

  return retval;
}
