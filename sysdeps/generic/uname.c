/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

/* This file is created by the configuration process, and defines UNAME_*.  */
#include <config-name.h>

/* Put information about the system in NAME.  */
int
DEFUN(uname, (name), struct utsname *name)
{
  int save;

  if (name == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  save = errno;
  if (__gethostname (name->nodename, sizeof (name->nodename)) < 0)
    {
      if (errno == ENOSYS)
	{
	  /* Hostname is meaningless for this machine.  */
	  name->nodename[0] = '\0';
	  errno = save;
	}
#ifdef	ENAMETOOLONG
      else if (errno == ENAMETOOLONG)
	/* The name was truncated.  */
	errno = save;
#endif
      else
	return -1;
    }
  strncpy (name->sysname, UNAME_SYSNAME, sizeof (name->sysname));
  strncpy (name->release, UNAME_RELEASE, sizeof (name->release));
  strncpy (name->version, UNAME_VERSION, sizeof (name->version));
  strncpy (name->machine, UNAME_MACHINE, sizeof (name->machine));

  return 0;
}
