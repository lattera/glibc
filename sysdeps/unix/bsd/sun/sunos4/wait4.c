/* This implements wait4 with the 4.4 BSD semantics (also those documented in
   SunOS 4.1) on top of SunOS's wait4 system call, which has semantics
   different from those documented.  Go Sun!

Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern pid_t __wait4_syscall __P ((pid_t pid, __WAIT_STATUS_DEFN stat_loc,
				   int options, struct rusage *usage));

pid_t
DEFUN(__wait4, (pid, stat_loc, options, usage),
      pid_t pid AND __WAIT_STATUS_DEFN stat_loc AND
      int options AND struct rusage *usage)
{
  switch (pid)
    {
    case WAIT_ANY:
      pid = 0;
      break;

    case WAIT_MYPGRP:
      pid = - __getpgrp (0);
      break;
    }

  return __wait4_syscall (pid, stat_loc, options, usage);
}
