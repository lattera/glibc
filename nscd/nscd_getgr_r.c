/* Copyright (C) 1998, 1999, 2000, 2002, 2003, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@uni-paderborn.de>, 1998.

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

#include <errno.h>
#include <grp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <not-cancel.h>

#include "nscd-client.h"
#include "nscd_proto.h"

int __nss_not_use_nscd_group;

static int nscd_getgr_r (const char *key, size_t keylen, request_type type,
			 struct group *resultbuf, char *buffer,
			 size_t buflen, struct group **result)
     internal_function;


int
__nscd_getgrnam_r (const char *name, struct group *resultbuf, char *buffer,
		   size_t buflen, struct group **result)
{
  return nscd_getgr_r (name, strlen (name) + 1, GETGRBYNAME, resultbuf,
		       buffer, buflen, result);
}


int
__nscd_getgrgid_r (gid_t gid, struct group *resultbuf, char *buffer,
		   size_t buflen, struct group **result)
{
  char buf[3 * sizeof (gid_t)];
  buf[sizeof (buf) - 1] = '\0';
  char *cp = _itoa_word (gid, buf + sizeof (buf) - 1, 10, 0);

  return nscd_getgr_r (cp, buf + sizeof (buf) - cp, GETGRBYGID, resultbuf,
		       buffer, buflen, result);
}


static int
internal_function
nscd_getgr_r (const char *key, size_t keylen, request_type type,
	      struct group *resultbuf, char *buffer, size_t buflen,
	      struct group **result)
{
  gr_response_header gr_resp;
  int sock = __nscd_open_socket (key, keylen, type, &gr_resp,
				 sizeof (gr_resp));
  if (sock == -1)
    {
      __nss_not_use_nscd_group = 1;
      return -1;
    }

  /* No value found so far.  */
  int retval = -1;
  *result = NULL;

  if (gr_resp.found == -1)
    {
      /* The daemon does not cache this database.  */
      __nss_not_use_nscd_group = 1;
      goto out;
    }

  if (gr_resp.found == 1)
    {
      struct iovec vec[2];
      uint32_t *len;
      char *p = buffer;
      size_t total_len;
      uintptr_t align;
      nscd_ssize_t cnt;

      /* Now allocate the buffer the array for the group members.  We must
	 align the pointer.  */
      align = ((__alignof__ (char *) - (p - ((char *) 0)))
	       & (__alignof__ (char *) - 1));
      total_len = align + (1 + gr_resp.gr_mem_cnt) * sizeof (char *)
		  + gr_resp.gr_name_len + gr_resp.gr_passwd_len;
      if (__builtin_expect (buflen < total_len, 0))
	{
	no_room:
	  __set_errno (ERANGE);
	  retval = ERANGE;
	  goto out;
	}
      buflen -= total_len;

      p += align;
      resultbuf->gr_mem = (char **) p;
      p += (1 + gr_resp.gr_mem_cnt) * sizeof (char *);

      /* Set pointers for strings.  */
      resultbuf->gr_name = p;
      p += gr_resp.gr_name_len;
      resultbuf->gr_passwd = p;
      p += gr_resp.gr_passwd_len;

      /* Fill in what we know now.  */
      resultbuf->gr_gid = gr_resp.gr_gid;

      /* Allocate array to store lengths.  */
      len = (uint32_t *) alloca (gr_resp.gr_mem_cnt * sizeof (uint32_t));

      total_len = gr_resp.gr_mem_cnt * sizeof (uint32_t);
      vec[0].iov_base = len;
      vec[0].iov_len = total_len;
      vec[1].iov_base = resultbuf->gr_name;
      vec[1].iov_len = gr_resp.gr_name_len + gr_resp.gr_passwd_len;
      total_len += gr_resp.gr_name_len + gr_resp.gr_passwd_len;

      /* Get this data.  */
      size_t n = TEMP_FAILURE_RETRY (__readv (sock, vec, 2));
      if (__builtin_expect (n != total_len, 0))
	goto out;

      /* Clear the terminating entry.  */
      resultbuf->gr_mem[gr_resp.gr_mem_cnt] = NULL;

      /* Prepare reading the group members.  */
      total_len = 0;
      for (cnt = 0; cnt < gr_resp.gr_mem_cnt; ++cnt)
	{
	  resultbuf->gr_mem[cnt] = p;
	  total_len += len[cnt];
	  p += len[cnt];
	}

      if (__builtin_expect (total_len > buflen, 0))
	goto no_room;

      retval = 0;
      n = TEMP_FAILURE_RETRY (__read (sock, resultbuf->gr_mem[0],
					     total_len));
      if (__builtin_expect (n != total_len, 0))
	{
	  /* The `errno' to some value != ERANGE.  */
	  __set_errno (ENOENT);
	  retval = ENOENT;
	}
      else
	*result = resultbuf;
    }
  else
    {
      /* The `errno' to some value != ERANGE.  */
      __set_errno (ENOENT);
      /* Even though we have not found anything, the result is zero.  */
      retval = 0;
    }

 out:
  close_not_cancel_no_status (sock);

  return retval;
}
