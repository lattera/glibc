/* Copyright (C) 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ian Lance Taylor (ian@airs.com).

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

/* On Ultrix we can use the getsysinfo call to get the right return
   value for _SC_CHILD_MAX.  Everything else is from <sys/param.h>,
   which the default sysconf already knows how to handle.  */

#include <unistd.h>
#include <errno.h>

/* This is an Ultrix header file.  */
#include <sys/sysinfo.h>

extern int __getsysinfo __P ((unsigned int op, void *buffer,
			      size_t nbytes, int *start, void *arg));
extern long int __default_sysconf __P ((int name));

long int
__sysconf (name)
     int name;
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
	  __set_errno (save);
	  return ret;
	}

      __set_errno (save);
    }

  return __default_sysconf (name);
}

#define __sysconf __default_sysconf

#include <sysdeps/posix/sysconf.c>
