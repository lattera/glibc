/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by C. Scott Ananian <cananian@alumni.princeton.edu>, 1998.

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

/* pt_chmod.c ... securely implement grantpt in user-land.  */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <grp.h>

#include "pty-internal.h"
#define Str(x) _Str(x)
#define _Str(x) #x

void
usage (void)
{
  fprintf (stderr, _("usage: pt_chown FD>&%s\n"
		     "This program is used internally by grantpt(3).\n"),
	   Str (PTY_FD));
  exit (0);
}

void
bad_installation (void)
{
  fputs (_("pt_chown: installation problem: "
	   "This program needs to be setuid root.\n"), stderr);
  exit (FAIL_EXEC);
}

int
main (int argc, char **argv)
{
  struct group *grp;
  struct stat s;
  char *pty;
  gid_t gid;
  uid_t uid;

  if (argc != 1)
    usage ();
  if (geteuid () != 0)
    bad_installation ();

  grp = getgrnam (TTY_GROUP);
  gid = grp ? grp->gr_gid : getgid ();
  uid = getuid ();

  /* Check that fd is a valid pty master -- call ptsname().  */
  pty = ptsname (PTY_FD);
  if (pty == NULL)
    return errno == EBADF ? FAIL_EBADF : FAIL_EINVAL;
  close (PTY_FD);

  /* Check that target file is a character device.  */
  if (stat (pty, &s))
    return FAIL_EINVAL; /* This should only fail if pty doesn't exist.  */
  if (!S_ISCHR (s.st_mode))
    return FAIL_EINVAL;

  if (chmod (pty, 0620))
    return FAIL_EACCES;  /* XXX: Probably not true. */

  if (chown (pty, uid, gid))
    return FAIL_EACCES;

  return 0;
}
