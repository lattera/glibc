/* Copyright (C) 1991, 1992, 1995, 1997, 2002 Free Software Foundation, Inc.
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

ENTRY (__pipe)
	mov %o0, %o2		/* Save PIPEDES. */
PSEUDO (__Spipe, pipe, 1)
	st %o0, [%o2]		/* PIPEDES[0] = %o0; */
	st %o1, [%o2 + 4]	/* PIPEDES[1] = %o1; */
	retl			/* return 0; */
	clr %o0

libc_hidden_def (__pipe)
weak_alias (__pipe, pipe)
