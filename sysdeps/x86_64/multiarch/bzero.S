/* bzero.  x86-64 version.
   Copyright (C) 2010-2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <init-arch.h>

	.text
ENTRY(__bzero)
	mov	%rsi,%rdx	/* Adjust parameter.  */
	xorl	%esi,%esi	/* Fill with 0s.  */
	jmp	__libc_memset	/* Branch to IFUNC memset.  */
END(__bzero)
weak_alias (__bzero, bzero)
