/* sigstack, sigaltstack definitions.
   Copyright (C) 1998 Free Software Foundation, Inc.
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


/* Structure describing a signal stack (obsolete).  */
struct sigstack
  {
    __ptr_t ss_sp;		/* Signal stack pointer.  */
    int ss_onstack;		/* Nonzero if executing on this stack.  */
  };


/* Alternate, preferred interface.  */
typedef struct sigaltstack
  {
    __ptr_t ss_sp;
    size_t ss_size;
    int ss_flags;
  } stack_t;
