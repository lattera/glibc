/* Copyright (C) 1991, 1992, 1997 Free Software Foundation, Inc.
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

#define DONT_LOAD_G1
#include <sysdep.h>
ENTRY (__libc_syscall)
	or	%o0,%g0,%g1
	or	%o1,%g0,%o0
	or	%o2,%g0,%o1
	or	%o3,%g0,%o2
	or	%o4,%g0,%o3
	or	%o5,%g0,%o4
PSEUDO_NOENT(__libc_syscall, syscall, 5)
	ret
SYSCALL__POST(syscall,5)
