/* sigstack, sigaltstack definitions.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _SIGNAL_H
# error "Never include this file directly.  Use <signal.h> instead"
#endif

#ifndef _SIGSTACK_H
#define _SIGSTACK_H	1

/* Structure describing a signal stack (obsolete).  */
struct sigstack
  {
    __ptr_t ss_sp;		/* Signal stack pointer.  */
    int ss_onstack;		/* Nonzero if executing on this stack.	*/
  };


/* Possible values for `ss_flags.'.  */
enum
{
  SS_ONSTACK = 1,
#define SS_ONSTACK	SS_ONSTACK
  SS_DISABLE
#define SS_DISABLE	SS_DISABLE
};

/* Minimum stack size for a signal handler.  */
#define MINSIGSTKSZ	2048

/* System default stack size.  */
#define SIGSTKSZ	8192


/* Alternate, preferred interface.  */
typedef struct sigaltstack
  {
    __ptr_t ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;

#endif	/* bits/sigstack.h */
