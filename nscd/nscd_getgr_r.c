/* Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include "nscd-client.h"
#include "nscd_proto.h"

int __nss_not_use_nscd_group;

static int nscd_getgr_r (const char *key, size_t keylen, request_type type,
			 struct group *resultbuf, char *buffer,
			 size_t buflen) internal_function;


int
__nscd_getgrnam_r (const char *name, struct group *resultbuf, char *buffer,
		   size_t buflen)
{
  return nscd_getgr_r (name, strlen (name) + 1, GETGRBYNAME, resultbuf,
		       buffer, buflen);
}


int
__nscd_getgrgid_r (gid_t gid, struct group *resultbuf, char *buffer,
		   size_t buflen)
{
  char buf[12];
  size_t n;

  n = __snprintf (buf, sizeof (buf), "%d", gid) + 1;

  return nscd_getgr_r (buf, n, GETGRBYGID, resultbuf, buffer, buflen);
}


/* Create a socket connected to a name. */
static int
open_socket (void)
{
  struct sockaddr_un addr;
  int sock;
  int saved_errno = errno;

  sock = __socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
      __set_errno (saved_errno);
      return -1;
    }

  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, _PATH_NSCDSOCKET);
  if (__connect (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
      __close (sock);
      __set_errno (saved_errno);
      return -1;
    }

  return sock;
}


static int
internal_function
nscd_getgr_r (const char *key, size_t keylen, request_type type,
	      struct group *resultbuf, char *buffer, size_t buflen)
{
  int sock = open_socket ();
  request_header req;
  gr_response_header gr_resp;
  ssize_t nbytes;
  struct iovec vec[2];

  if (sock == -1)
    {
      __nss_not_use_nscd_group = 1;
      return -1;
    }

  req.version = NSCD_VERSION;
  req.type = type;
  req.key_len = keylen;

  vec[0].iov_base = &req;
  vec[0].iov_len = sizeof (request_header);
  vec[1].iov_base = (void *) key;
  vec[1].iov_len = keylen;

  if (__writev (sock, vec, 2) != sizeof (request_header) + keylen)
    {
      __close (sock);
      return -1;
    }

  nbytes = __read (sock, &gr_resp, sizeof (gr_response_header));
  if (nbytes != sizeof (gr_response_header))
    {
      __close (sock);
      return -1;
    }

  if (gr_resp.found == -1)
    {
      /* The daemon does not cache this database.  */
      __close (sock);
      __nss_not_use_nscd_group = 1;
      return -1;
    }

  if (gr_resp.found == 1)
    {
      uint32_t *len;
      char *p = buffer;
      size_t total_len;
      uintptr_t align;
      size_t cnt;

      /* Now allocate the buffer the array for the group members.  We must
	 align the pointer.  */
      align = ((__alignof__ (char *) - (p - ((char *) 0)))
	       & (__alignof__ (char *) - 1));
      total_len = align + (1 + gr_resp.gr_mem_cnt) * sizeof (char *)
		  + gr_resp.gr_name_len + gr_resp.gr_passwd_len;
      if (buflen < total_len)
	{
	no_room:
	  __set_errno (ERANGE);
	  __close (sock);
	  return ERANGE;
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
      if (__readv (sock, vec, 2) != total_len)
	{
	  __close (sock);
	  return -1;
	}

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

      if (total_len > buflen)
	goto no_room;

      if (__read (sock, resultbuf->gr_mem[0], total_len) != total_len)
	{
	  __close (sock);
	  /* The `errno' to some value != ERANGE.  */
	  __set_errno (ENOENT);
	  return ENOENT;
	}

      __close (sock);
      return 0;
    }
  else
    {
      __close (sock);
      /* The `errno' to some value != ERANGE.  */
      __set_errno (ENOENT);
      return ENOENT;
    }
}
