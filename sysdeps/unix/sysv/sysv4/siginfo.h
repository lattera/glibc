/* Definitions of the siginfo structure.
   Copyright (C) 1993, 1994, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#ifndef	_SIGINFO_H
#define	_SIGINFO_H	1

#ifdef __USE_SVID
/* SVR4 puts a ton of other stuff in this structure.  For now, we'll just
   define the two things we really need out of it, and hope for the best.  */

/* These define the different states a child can have on exit.
   We need these to build the status return for things like waitpid.  */
#define EXITED 		1
#define KILLED		2
#define CORED		3
#define TRAPPED		4
#define STOPPED		5
#define CONTINUED	6

typedef struct __siginfo
  {
    int filler1;

    /* Code indicating child's status */
    int __code;

    int filler2;

    /* The PID of the child.  */
    long __pid;

    int filler3;

    /* The child's status.  */
    int __status;

    int filler4[26];

  } __siginfo_t;

#endif  /* __USE_SVID */
#endif	/* siginfo.h */
