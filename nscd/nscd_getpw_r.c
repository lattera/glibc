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
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "nscd.h"

int __nss_not_use_nscd_passwd;

static int __nscd_getpw_r (const char *key, request_type type,
			   struct passwd *resultbuf, char *buffer,
			   size_t buflen);

int
__nscd_getpwnam_r (const char *name, struct passwd *resultbuf, char *buffer,
		   size_t buflen)
{
  if (name == NULL)
    return 1;

  return __nscd_getpw_r (name, GETPWBYNAME, resultbuf, buffer, buflen);
}

int
__nscd_getpwuid_r (uid_t uid, struct passwd *resultbuf, char *buffer,
		   size_t buflen)
{
  char *p = buffer;
  int plen;

  plen = __snprintf (buffer, buflen, "%d", uid);
  if (plen == -1)
    {
      __set_errno (ERANGE);
      return -1;
    }
  p = buffer + plen + 1;

  return __nscd_getpw_r (buffer, GETPWBYUID, resultbuf, p, buflen - plen -1);
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
      close (sock);
      __set_errno (saved_errno);
      return -1;
    }

  return sock;
}

static int
__nscd_getpw_r (const char *key, request_type type, struct passwd *resultbuf,
		char *buffer, size_t buflen)
{
  int sock = nscd_open_socket ();
  request_header req;
  pw_response_header pw_resp;
  ssize_t nbytes;

  if (sock == -1)
    {
      __nss_not_use_nscd_passwd = 1;
      return 1;
    }

  req.version = NSCD_VERSION;
  req.type = type;
  req.key_len = strlen (key);
  nbytes = write (sock, &req, sizeof (request_header));
  if (nbytes != sizeof (request_header))
    {
      close (sock);
      return 1;
    }

  nbytes = write (sock, key, req.key_len);
  if (nbytes != req.key_len)
    {
      close (sock);
      return 1;
    }

  nbytes = read (sock, &pw_resp, sizeof (pw_response_header));
  if (nbytes != sizeof (pw_response_header))
    {
      close (sock);
      return 1;
    }

  if (pw_resp.found == -1)
    {
      /* The daemon does not cache this database.  */
      close (sock);
      __nss_not_use_nscd_passwd = 1;
      return 1;
    }

  if (pw_resp.found == 1)
    {
      struct iovec vec[5];
      char *p = buffer;

      if (buflen < (pw_resp.pw_name_len + 1 + pw_resp.pw_passwd_len + 1
		    + pw_resp.pw_gecos_len + 1 + pw_resp.pw_dir_len + 1
		    + pw_resp.pw_shell_len + 1))
	{
	  __set_errno (ERANGE);
	  close (sock);
	  return -1;
	}

      /* get pw_name */
      vec[0].iov_base = p;
      vec[0].iov_len = pw_resp.pw_name_len;
      p += pw_resp.pw_name_len + 1;
      buflen -= (pw_resp.pw_name_len + 1);
      /* get pw_passwd */
      vec[1].iov_base = p;
      vec[1].iov_len = pw_resp.pw_passwd_len;
      p += pw_resp.pw_passwd_len + 1;
      buflen -= (pw_resp.pw_passwd_len + 1);
      /* get pw_gecos */
      vec[2].iov_base = p;
      vec[2].iov_len = pw_resp.pw_gecos_len;
      p += pw_resp.pw_gecos_len + 1;
      buflen -= (pw_resp.pw_gecos_len + 1);
      /* get pw_dir */
      vec[3].iov_base = p;
      vec[3].iov_len = pw_resp.pw_dir_len;
      p += pw_resp.pw_dir_len + 1;
      buflen -= (pw_resp.pw_dir_len + 1);
      /* get pw_pshell */
      vec[4].iov_base = p;
      vec[4].iov_len = pw_resp.pw_shell_len;
      p += pw_resp.pw_shell_len + 1;
      buflen -= (pw_resp.pw_shell_len + 1);

      nbytes = __readv (sock, vec, 5);
      if (nbytes !=  (pw_resp.pw_name_len + pw_resp.pw_passwd_len
		      + pw_resp.pw_gecos_len + pw_resp.pw_dir_len
		      + pw_resp.pw_shell_len))
	{
	  close (sock);
	  return 1;
	}

      resultbuf->pw_name = vec[0].iov_base;
      resultbuf->pw_name[pw_resp.pw_name_len] = '\0';
      resultbuf->pw_passwd = vec[1].iov_base;
      resultbuf->pw_passwd[pw_resp.pw_passwd_len] = '\0';
      resultbuf->pw_uid = pw_resp.pw_uid;
      resultbuf->pw_gid = pw_resp.pw_gid;
      resultbuf->pw_gecos = vec[2].iov_base;
      resultbuf->pw_gecos[pw_resp.pw_gecos_len] = '\0';
      resultbuf->pw_dir = vec[3].iov_base;
      resultbuf->pw_dir[pw_resp.pw_dir_len] = '\0';
      resultbuf->pw_shell = vec[4].iov_base;
      resultbuf->pw_shell[pw_resp.pw_shell_len] = '\0';

      close (sock);
      return 0;
    }
  else
    {
      close (sock);
      return -1;
    }
}
