/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@uni-paderborn.de>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "nscd.h"
#include "nscd_proto.h"

int __nss_not_use_nscd_group;

static int __nscd_getgr_r (const char *key, request_type type,
			   struct group *resultbuf, char *buffer,
			   size_t buflen);

int
__nscd_getgrnam_r (const char *name, struct group *resultbuf, char *buffer,
		   size_t buflen)
{
  if (name == NULL)
    return 1;

  return __nscd_getgr_r (name, GETGRBYNAME, resultbuf, buffer, buflen);
}

int
__nscd_getgrgid_r (gid_t gid, struct group *resultbuf, char *buffer,
		   size_t buflen)
{
  char *p = buffer;
  int plen;

  plen = __snprintf (buffer, buflen, "%d", gid);
  if (plen == -1)
    {
      __set_errno (ERANGE);
      return -1;
    }
  p = buffer + plen + 1;

  return __nscd_getgr_r (buffer, GETGRBYGID, resultbuf, p, buflen - plen -1);
}

/* Create a socket connected to a name. */
static int
nscd_open_socket (void)
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
__nscd_getgr_r (const char *key, request_type type, struct group *resultbuf,
		char *buffer, size_t buflen)
{
  int sock = nscd_open_socket ();
  request_header req;
  gr_response_header gr_resp;
  ssize_t nbytes;

  if (sock == -1)
    {
      __nss_not_use_nscd_group = 1;
      return 1;
    }

  req.version = NSCD_VERSION;
  req.type = type;
  req.key_len = strlen (key);
  nbytes = __write (sock, &req, sizeof (request_header));
  if (nbytes != sizeof (request_header))
    {
      __close (sock);
      return 1;
    }

  nbytes = __write (sock, key, req.key_len);
  if (nbytes != req.key_len)
    {
      __close (sock);
      return 1;
    }

  nbytes = __read (sock, &gr_resp, sizeof (gr_response_header));
  if (nbytes != sizeof (gr_response_header))
    {
      __close (sock);
      return 1;
    }

  if (gr_resp.found == -1)
    {
      /* The daemon does not cache this database.  */
      __close (sock);
      __nss_not_use_nscd_group = 1;
      return 1;
    }

  if (gr_resp.found == 1)
    {
      struct iovec *vec;
      size_t *len;
      char *p = buffer;
      int nblocks;
      size_t total_len;
      uintptr_t align;

      /* A first check whether the buffer is sufficently large is possible.  */
      if (buflen < gr_resp.gr_name_len + 1 + gr_resp.gr_passwd_len + 1)
	{
	  __set_errno (ERANGE);
	  __close (sock);
	  return -1;
	}

      /* Allocate the IOVEC.  */
      vec = alloca ((2 + gr_resp.gr_mem_len) * sizeof (struct iovec));
      len = alloca (gr_resp.gr_mem_len * sizeof (size_t));

      vec[0].iov_base = resultbuf->gr_name = p;
      vec[0].iov_len = gr_resp.gr_name_len;
      total_len = gr_resp.gr_name_len;
      p += gr_resp.gr_name_len + 1;

      vec[1].iov_base = resultbuf->gr_passwd = p;
      vec[1].iov_len = gr_resp.gr_passwd_len;
      total_len += gr_resp.gr_passwd_len;
      p += gr_resp.gr_passwd_len + 1;
      buflen -= total_len;
      nblocks = 2;

      if (gr_resp.gr_mem_len > 0)
	{
	  vec[2].iov_base = len;
	  vec[2].iov_len = gr_resp.gr_mem_len * sizeof (size_t);
	  total_len += gr_resp.gr_mem_len * sizeof (size_t);
	  nblocks = 3;
	}

      /* Get this data.  */
      if (__readv (sock, vec, nblocks) != total_len)
	{
	  __close (sock);
	  return 1;
	}

      /* Now we know the sizes.  First terminate the strings we just read. */
      resultbuf->gr_name[gr_resp.gr_name_len] = '\0';
      resultbuf->gr_passwd[gr_resp.gr_passwd_len] = '\0';

      resultbuf->gr_gid = gr_resp.gr_gid;

      /* Now allocate the buffer the array for the group members.  We must
	 align the pointer.  */
      align = ((__alignof__ (char *) - (p - ((char *) 0)))
	       & (__alignof__ (char *) - 1));
      if (align + (1 + gr_resp.gr_mem_len) * sizeof (char *) > buflen)
	{
	  __set_errno (ERANGE);
	  __close (sock);
	  return -1;
	}
      p += align;
      resultbuf->gr_mem = (char **) p;
      p += (1 + gr_resp.gr_mem_len) * sizeof (char *);
      buflen -= align + (1 + gr_resp.gr_mem_len) * sizeof (char *);

      resultbuf->gr_mem[gr_resp.gr_mem_len] = NULL;

      if (gr_resp.gr_mem_len > 0)
	{
	  /* Prepare reading the group members.  */
	  size_t i;

	  total_len = 0;
	  for (i = 0; i < gr_resp.gr_mem_len; ++i)
	    {
	      if (len[i] >= buflen)
		{
		  __set_errno (ERANGE);
		  __close (sock);
		  return -1;
		}

	      vec[i].iov_base = resultbuf->gr_mem[i] = p;
	      vec[i].iov_len = len[i];
	      total_len += len[i];
	      buflen -= len[i];
	      p += len[i];
	      *p++ = '\0';
	    }

	  if (__readv (sock, vec, gr_resp.gr_mem_len) != total_len)
	    {
	      __close (sock);
	      return -1;
	    }
	}
      __close (sock);
      return 0;
    }
  else
    {
      __close (sock);
      return -1;
    }
}
