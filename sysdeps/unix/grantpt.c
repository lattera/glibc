/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

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
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>

#include "pty-internal.h"

/* Given a fd on a master pseudoterminal, chown the file associated
   with the slave to the calling process, and set its group and
   mode appropriately.  Note that this is an unprivileged operation. */

/* This "generic Unix" implementation works because we provide the program
   /usr/libexec/pt_chown, and it only depends on ptsname() working. */
static const char helper[] = LIBEXECDIR "/pt_chown";
static const char *const argv[] = { "pt_chown", NULL };

int
grantpt (fd)
     int fd;
{
  struct stat st;
  int w, pid;
  char namebuf[PTYNAMELEN];

  /* Some systems do it for us.  */
  if (__ptsname_r (fd, namebuf, PTYNAMELEN) != 0)
    return -1;
  if (__xstat (_STAT_VER, namebuf, &st) != 0)
    return -1;

  if (st.st_uid == __getuid ())
    return 0;

  /* We have to do it in user space.  */

  pid = __fork ();
  if (pid == -1)
    return -1;
  else if (pid == 0)
    {
      /* Disable core dumps in the child.  */
      struct rlimit off = { 0, 0 };
      setrlimit (RLIMIT_CORE, &off);

      /* The helper does its thing on fd PTY_FD.  */
      if (fd != PTY_FD)
	if (__dup2 (fd, PTY_FD) == -1)
	  _exit (FAIL_EBADF);

      __execve (helper, (char *const *) argv, 0);
      _exit (FAIL_EXEC);
    }
  else
    {
      if (__waitpid (pid, &w, 0) == -1)
	return -1;
      if (!WIFEXITED (w))
	{
	  __set_errno (ENOEXEC);
	  return -1;
	}
      else
	switch (WEXITSTATUS(w))
	  {
	  case 0:
	    break;
	  case FAIL_EBADF:
	    __set_errno (EBADF);
	    return -1;
	  case FAIL_EINVAL:
	    __set_errno (EINVAL);
	    return -1;
	  case FAIL_EACCES:
	    __set_errno (EACCES);
	    return -1;
	  case FAIL_EXEC:
	    __set_errno (ENOEXEC);
	    return -1;

	  default:
	    assert(! "getpt: internal error: invalid exit code from pt_chown");
	  }
    }

  /* Success.  */
  return 0;
}
