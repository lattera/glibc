/* Copyright (C) 1992 Free Software Foundation, Inc.
   Contributed by Ian Lance Taylor (ian@airs.com).

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

/* On Ultrix we can use the getsysinfo call to get the right return
   value for _SC_CHILD_MAX.  Everything else is from <sys/param.h>,
   which the default sysconf already knows how to handle.  */

#include <ansidecl.h>
#include <unistd.h>
#include <errno.h>

/* This is an Ultrix header file.  */
#include <sys/sysinfo.h>

extern int EXFUN(__getsysinfo, (unsigned int op, void *buffer,
				size_t nbytes, int *start,
				void *arg));
extern long int EXFUN(__default_sysconf, (int name));

long int
DEFUN(__sysconf, (name), int name)
{
  if (name == _SC_CHILD_MAX)
    {
      int save = errno;
      int start = 0;
      int ret;

      /* getsysinfo returns the number of values it put into the
	 buffer, or 0 if not available, or -1 on error.  */
      if (__getsysinfo (GSI_MAX_UPROCS, &ret, sizeof (ret), &start,
			(void *) 0) > 0)
	{
	  errno = save;
	  return ret;
	}

      errno = save;
    }

  return __default_sysconf (name);
}

#define __sysconf __default_sysconf

#include <sysdeps/posix/__sysconf.c>
