/* Copyright (C) 2010-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <pwd.h>
#include <unistd.h>
#include <not-cancel.h>

#define STATIC static
static int getlogin_r_fd0 (char *name, size_t namesize);
#define __getlogin_r getlogin_r_fd0
#include <sysdeps/unix/getlogin_r.c>
#undef __getlogin_r


/* Try to determine login name from /proc/self/loginuid and return 0
   if successful.  If /proc/self/loginuid cannot be read return -1.
   Otherwise return the error number.  */

int
attribute_hidden
__getlogin_r_loginuid (char *name, size_t namesize)
{
  int fd = __open_nocancel ("/proc/self/loginuid", O_RDONLY);
  if (fd == -1)
    return -1;

  /* We are reading a 32-bit number.  12 bytes are enough for the text
     representation.  If not, something is wrong.  */
  char uidbuf[12];
  ssize_t n = TEMP_FAILURE_RETRY (__read_nocancel (fd, uidbuf,
						   sizeof (uidbuf)));
  __close_nocancel_nostatus (fd);

  uid_t uid;
  char *endp;
  if (n <= 0
      || n == sizeof (uidbuf)
      || (uidbuf[n] = '\0',
	  uid = strtoul (uidbuf, &endp, 10),
	  endp == uidbuf || *endp != '\0'))
    return -1;

  size_t buflen = 1024;
  char *buf = alloca (buflen);
  bool use_malloc = false;
  struct passwd pwd;
  struct passwd *tpwd;
  int result = 0;
  int res;

  while ((res = __getpwuid_r (uid, &pwd, buf, buflen, &tpwd)) == ERANGE)
    if (__libc_use_alloca (2 * buflen))
      buf = extend_alloca (buf, buflen, 2 * buflen);
    else
      {
	buflen *= 2;
	char *newp = realloc (use_malloc ? buf : NULL, buflen);
	if (newp == NULL)
	  {
	    result = ENOMEM;
	    goto out;
	  }
	buf = newp;
	use_malloc = true;
      }

  if (res != 0 || tpwd == NULL)
    {
      result = -1;
      goto out;
    }

  size_t needed = strlen (pwd.pw_name) + 1;
  if (needed > namesize)
    {
      __set_errno (ERANGE);
      result = ERANGE;
      goto out;
    }

  memcpy (name, pwd.pw_name, needed);

 out:
  if (use_malloc)
    free (buf);

  return result;
}


/* Return at most NAME_LEN characters of the login name of the user in NAME.
   If it cannot be determined or some other error occurred, return the error
   code.  Otherwise return 0.  */

int
__getlogin_r (char *name, size_t namesize)
{
  int res = __getlogin_r_loginuid (name, namesize);
  if (res >= 0)
    return res;

  return getlogin_r_fd0 (name, namesize);
}
libc_hidden_def (__getlogin_r)
weak_alias (__getlogin_r, getlogin_r)
libc_hidden_weak (getlogin_r)
