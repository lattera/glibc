/* Copyright (C) 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <pwd.h>
#include <unistd.h>
#include <not-cancel.h>

#define STATIC static
static int getlogin_r_fd0 (char *name, size_t namesize);
#define getlogin_r getlogin_r_fd0
#include <sysdeps/unix/getlogin_r.c>
#undef getlogin_r


int
attribute_hidden
__getlogin_r_loginuid (name, namesize)
     char *name;
     size_t namesize;
{
  int fd = open_not_cancel_2 ("/proc/self/loginuid", O_RDONLY);
  if (fd == -1)
    return 1;

  ssize_t n = TEMP_FAILURE_RETRY (read_not_cancel (fd, name, namesize));
  close_not_cancel_no_status (fd);

  uid_t uid;
  char *endp;
  if (n <= 0
      || (uid = strtoul (name, &endp, 10), endp == name || *endp != '\0'))
    return 1;

  size_t buflen = 1024;
  char *buf = alloca (buflen);
  bool use_malloc = false;
  struct passwd pwd;
  struct passwd *tpwd;
  int res;

  while ((res = __getpwuid_r (uid, &pwd, buf, buflen, &tpwd)) != 0)
    if (__libc_use_alloca (2 * buflen))
      extend_alloca (buf, buflen, 2 * buflen);
    else
      {
	buflen *= 2;
	char *newp = realloc (use_malloc ? buf : NULL, buflen);
	if (newp == NULL)
	  {
	  fail:
	    if (use_malloc)
	      free (buf);
	    return 1;
	  }
	buf = newp;
	use_malloc = true;
      }

  if (tpwd == NULL)
    goto fail;

  strncpy (name, pwd.pw_name, namesize - 1);
  name[namesize - 1] = '\0';

  if (use_malloc)
    free (buf);

  return 0;
}


/* Return the login name of the user, or NULL if it can't be determined.
   The returned pointer, if not NULL, is good only until the next call.  */

int
getlogin_r (name, namesize)
     char *name;
     size_t namesize;
{
  if (__getlogin_r_loginuid (name, namesize) == 0)
    return 0;

  return getlogin_r_fd0 (name, namesize);
}
libc_hidden_def (getlogin_r)
