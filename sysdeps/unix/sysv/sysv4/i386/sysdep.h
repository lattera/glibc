/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
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

#include <sysdeps/unix/sysv/i386/sysdep.h>

/* In SVR4 some system calls can fail with the error ERESTART,
   and this means the call should be retried.  */

#ifndef _ERRNO_H
#define _ERRNO_H
#endif
#include <bits/errno.h>

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
