/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

/* This code wants to be run through m4; see sysdeps/m68k/Makefile.  */

#include <sysdeps/unix/sysdep.h>

#define	POUND(foo)	(@@@Hash-Here@@@)foo

#ifdef	__STDC__
#define	ENTRY(name)							      \
  .globl _##name;							      \
  .even;								      \
  _##name##:
#else
#define	ENTRY(name)							      \
  .globl _/**/name;							      \
  .even;								      \
  _/**/name/**/:
#endif

#ifdef	__STDC__
#define	PSEUDO(name, syscall_name, args)				      \
  .even;								      \
  .globl syscall_error;							      \
  error: jmp syscall_error;						      \
  ENTRY (name)								      \
  pea SYS_##syscall_name;						      \
  trap POUND(0);							      \
  bcs error
#else
#define	PSEUDO(name, syscall_name, args)				      \
  .even;								      \
  .globl syscall_error;							      \
  error: jmp syscall_error;						      \
  ENTRY (name)								      \
  pea SYS_/**/syscall_name;						      \
  trap POUND(0);							      \
  bcs error
#endif

#define	ret	rts
#define	r0	d0
#define	r1	d1
#define	MOVE(x,y)	movel x , y
