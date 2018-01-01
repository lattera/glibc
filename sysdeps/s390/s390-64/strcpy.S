/* strcpy - copy a string from source to destination.  64 bit S/390 version.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

/* INPUT PARAMETERS
     %r2 = address of destination
     %r3 = address of source.  */

#include "sysdep.h"
#include "asm-syntax.h"

        .text
ENTRY(strcpy)
	slgr  %r0,%r0
	lgr   %r1,%r2
0:	mvst  %r1,%r3
	jo    0b
	br    %r14
END(strcpy)
libc_hidden_builtin_def (strcpy)
