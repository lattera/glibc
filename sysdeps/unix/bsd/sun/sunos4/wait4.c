/* This implements wait4 with the 4.4 BSD semantics (also those documented in
   SunOS 4.1) on top of SunOS's wait4 system call, which has semantics
   different from those documented.  Go Sun!
   Copyright (C) 1991,1992,1993,1995,1997,2004 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern pid_t __wait4_syscall (pid_t pid, __WAIT_STATUS_DEFN stat_loc,
			      int options, struct rusage *usage);

pid_t
__wait4 (pid, stat_loc, options, usage)
     pid_t pid;
     __WAIT_STATUS_DEFN stat_loc;
     int options;
     struct rusage *usage;
{
  switch (pid)
    {
    case WAIT_ANY:
      pid = 0;
      break;

    case WAIT_MYPGRP:
      pid = - getpgrp ();
      break;
    }

  return __wait4_syscall (pid, stat_loc, options, usage);
}

weak_alias (__wait4, wait4)
