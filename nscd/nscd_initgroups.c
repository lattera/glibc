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
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <not-cancel.h>

#include "nscd-client.h"
#include "nscd_proto.h"


libc_locked_map_ptr (map_handle);
/* Note that we only free the structure if necessary.  The memory
   mapping is not removed since it is not visible to the malloc
   handling.  */
libc_freeres_fn (gr_map_free)
{
  if (map_handle.mapped != NO_MAPPING)
    free (map_handle.mapped);
}


int
__nscd_getgrouplist (const char *user, gid_t group, long int *size,
		     gid_t **groupsp, long int limit)
{
  size_t userlen = strlen (user) + 1;
  int gc_cycle;

  /* If the mapping is available, try to search there instead of
     communicating with the nscd.  */
  struct mapped_database *mapped;
  mapped = __nscd_get_map_ref (GETFDGR, "group", &map_handle, &gc_cycle);

 retry:;
  const initgr_response_header *initgr_resp = NULL;
  char *respdata = NULL;
  int retval = -1;
  int sock = -1;

  if (mapped != NO_MAPPING)
    {
      const struct datahead *found = __nscd_cache_search (INITGROUPS, user,
							  userlen, mapped);
      if (found != NULL)
	{
	  initgr_resp = &found->data[0].initgrdata;
	  respdata = (char *) (initgr_resp + 1);
	  char *recend = (char *) found->data + found->recsize;

	  if (respdata + initgr_resp->ngrps * sizeof (int32_t) > recend)
	    goto out;
	}
    }

  /* If we do not have the cache mapped, try to get the data over the
     socket.  */
  initgr_response_header initgr_resp_mem;
  if (initgr_resp == NULL)
    {
      sock = __nscd_open_socket (user, userlen, INITGROUPS, &initgr_resp_mem,
				 sizeof (initgr_resp_mem));
      if (sock == -1)
	/* nscd not running or wrong version or hosts caching disabled.  */
	__nss_not_use_nscd_group = 1;

      initgr_resp = &initgr_resp_mem;
    }

  if (initgr_resp->found == 1)
    {
      /* The following code assumes that gid_t and int32_t are the
	 same size.  This is the case for al existing implementation.
	 If this should change some code needs to be added which
	 doesn't use memcpy but instead copies each array element one
	 by one.  */
      assert (sizeof (int32_t) == sizeof (gid_t));
      assert (initgr_resp->ngrps > 0);

      /* Make sure we have enough room.  We always count GROUP in even
	 though we might not end up adding it.  */
      if (*size < initgr_resp->ngrps + 1)
	{
	  gid_t *newp = realloc (*groupsp,
				 (initgr_resp->ngrps + 1) * sizeof (gid_t));
	  if (newp == NULL)
	    /* We cannot increase the buffer size.  */
	    goto out;

	  *groupsp = newp;
	  *size = initgr_resp->ngrps + 1;
	}

      if (respdata == NULL)
	{
	  /* Read the data from the socket.  */
	  if ((size_t) TEMP_FAILURE_RETRY (__read (sock, *groupsp,
						   initgr_resp->ngrps
						   * sizeof (gid_t)))
	      == initgr_resp->ngrps * sizeof (gid_t))
	    retval = initgr_resp->ngrps;
	}
      else
	{
	  /* Just copy the data.  */
	  retval = initgr_resp->ngrps;
	  memcpy (*groupsp, respdata, retval * sizeof (gid_t));
	}
    }
  else
    {
      /* No group found yet.   */
      retval = 0;

      assert (*size >= 1);
    }

  /* Check whether GROUP is part of the mix.  If not, add it.  */
  if (retval >= 0)
    {
      int cnt;
      for (cnt = 0; cnt < retval; ++cnt)
	if ((*groupsp)[cnt] == group)
	  break;

      if (cnt == retval)
	(*groupsp)[retval++] = group;
    }

  if (sock != -1)
    close_not_cancel_no_status (sock);
 out:
  if (__nscd_drop_map_ref (mapped, &gc_cycle) != 0 && retval != -1)
    {
      /* When we come here this means there has been a GC cycle while we
	 were looking for the data.  This means the data might have been
	 inconsistent.  Retry if possible.  */
      if ((gc_cycle & 1) != 0)
	{
	  /* nscd is just running gc now.  Disable using the mapping.  */
	  __nscd_unmap (mapped);
	  mapped = NO_MAPPING;
	}

      goto retry;
    }

  return retval;
}
