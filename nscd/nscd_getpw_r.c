/* Copyright (C) 1998, 1999 Free Software Foundation, Inc.
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
#include <pwd.h>
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

int __nss_not_use_nscd_passwd;

static int nscd_getpw_r (const char *key, size_t keylen, request_type type,
			 struct passwd *resultbuf, char *buffer,
			 size_t buflen) internal_function;

int
__nscd_getpwnam_r (const char *name, struct passwd *resultbuf, char *buffer,
		   size_t buflen)
{
  if (name == NULL)
    return -1;

  return nscd_getpw_r (name, strlen (name) + 1, GETPWBYNAME, resultbuf,
		       buffer, buflen);
}

int
__nscd_getpwuid_r (uid_t uid, struct passwd *resultbuf, char *buffer,
		   size_t buflen)
{
  char buf[12];
  size_t n;

  n = __snprintf (buf, sizeof (buf), "%d", uid) + 1;

  return nscd_getpw_r (buf, n, GETPWBYUID, resultbuf, buffer, buflen);
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
nscd_getpw_r (const char *key, size_t keylen, request_type type,
	      struct passwd *resultbuf, char *buffer, size_t buflen)
{
  int sock = open_socket ();
  request_header req;
  pw_response_header pw_resp;
  ssize_t nbytes;

  if (sock == -1)
    {
      __nss_not_use_nscd_passwd = 1;
      return -1;
    }

  req.version = NSCD_VERSION;
  req.type = type;
  req.key_len = keylen;
  nbytes = __write (sock, &req, sizeof (request_header));
  if (nbytes != sizeof (request_header))
    {
      __close (sock);
      return -1;
    }

  nbytes = __write (sock, key, keylen);
  if (nbytes != keylen)
    {
      __close (sock);
      return -1;
    }

  nbytes = __read (sock, &pw_resp, sizeof (pw_response_header));
  if (nbytes != sizeof (pw_response_header))
    {
      __close (sock);
      return -1;
    }

  if (pw_resp.found == -1)
    {
      /* The daemon does not cache this database.  */
      __close (sock);
      __nss_not_use_nscd_passwd = 1;
      return -1;
    }

  if (pw_resp.found == 1)
    {
      char *p = buffer;
      size_t total = (pw_resp.pw_name_len + pw_resp.pw_passwd_len
		      + pw_resp.pw_gecos_len + pw_resp.pw_dir_len
		      + pw_resp.pw_shell_len);

      if (buflen < total)
	{
	  __set_errno (ERANGE);
	  __close (sock);
	  return ERANGE;
	}

      /* Set the information we already have.  */
      resultbuf->pw_uid = pw_resp.pw_uid;
      resultbuf->pw_gid = pw_resp.pw_gid;

      /* get pw_name */
      resultbuf->pw_name = p;
      p += pw_resp.pw_name_len;
      /* get pw_passwd */
      resultbuf->pw_passwd = p;
      p += pw_resp.pw_passwd_len;
      /* get pw_gecos */
      resultbuf->pw_gecos = p;
      p += pw_resp.pw_gecos_len;
      /* get pw_dir */
      resultbuf->pw_dir = p;
      p += pw_resp.pw_dir_len;
      /* get pw_pshell */
      resultbuf->pw_shell = p;

      nbytes = __read (sock, buffer, total);

      __close (sock);

      return nbytes == total ? 0 : -1;
    }
  else
    {
      __close (sock);
      /* The `errno' to some value != ERANGE.  */
      __set_errno (ENOENT);
      return ENOENT;
    }
}
