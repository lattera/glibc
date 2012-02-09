/* strcmp - compare two string.  64 bit S/390 version.
   This file is part of the GNU C Library.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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

/* INPUT PARAMETERS
     %r2 = address of string 1
     %r3 = address of string 2.  */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(strcmp)
        slr   %r0,%r0
0:	clst  %r2,%r3
	jo    0b
	jp    1f
	jm    2f
	slgr  %r2,%r2
	br    %r14
1:	lghi  %r2,1
	br    %r14
2:	lghi  %r2,-1
	br    %r14
END(strcmp)
libc_hidden_builtin_def (strcmp)
