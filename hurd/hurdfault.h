/* Declarations for handling faults in the signal thread.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#ifndef _HURD_FAULT_H
#define _HURD_FAULT_H

#include <setjmp.h>

/* Call this before code that might fault in the signal thread; SIGNO is
   the signal expected to possibly arrive.  This behaves like setjmp: it
   returns zero the first time, and returns again nonzero if the signal
   does arrive.  */

#define _hurdsig_catch_fault(signo) \
  (_hurdsig_fault_expect_signo = (signo), setjmp (_hurdsig_fault_env))

/* Call this at the end of a section protected by _hurdsig_catch_fault.  */

#define _hurdsig_end_catch_fault() \
  (_hurdsig_fault_expect_signo = 0)

extern jmp_buf _hurdsig_fault_env;
extern int _hurdsig_fault_expect_signo;

/* If _hurdsig_catch_fault returns nonzero, these variables
   contain information about the signal that arrived.  */



extern long int _hurdsig_fault_sigcode;
extern int _hurdsig_fault_sigerror;

#endif	/* hurd/fault.h */
