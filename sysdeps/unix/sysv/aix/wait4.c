/* Copyright (C) 1991, 1992, 1995-1997, 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>

extern pid_t kwaitpid (int *stat_loc, pid_t pid, int options,
		       struct rusage *ru_loc, siginfo_t *infop);

pid_t
__wait4 (__pid_t pid, __WAIT_STATUS stat_loc, int options,
	 struct rusage *usage)
{
  return kwaitpid (stat_loc.__iptr, pid, options, usage, NULL);
}
strong_alias (__wait4, wait4)
