/* Definitions of the siginfo structure.
   Copyright (C) 1993 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#ifndef	_SIGINFO_H
#define	_SIGINFO_H	1

#ifdef __USE_SVID
/* SVR4 puts a ton of other stuff in this structure.  For now, we'll just
   define the two things we really need out of it, and hope for the best.  */

typedef struct __siginfo
{
  int filler1[3];

  /* The PID of the child.  */
  __pid_t __pid;

  int filler2;

  /* The child's status.  */
  int __status;

  int filler3[26];

} __siginfo_t;

#endif  /* __USE_SVID */
#endif	/* siginfo.h */
