/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <sysdeps/unix/sysv/i386/sysdep.h>

/* In SVR4 some system calls can fail with the error ERESTART,
   and this means the call should be retried.  */

#ifndef _ERRNO_H
#define _ERRNO_H
#endif
#include <errnos.h>

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .globl syscall_error;							      \
  ENTRY (name)								      \
  DO_CALL (syscall_name, args);						      \
  jae noerror;								      \
  cmpb $ERESTART, %al;							      \
  je C_SYMBOL_NAME (name);						      \
  jmp syscall_error;							      \
  noerror:
