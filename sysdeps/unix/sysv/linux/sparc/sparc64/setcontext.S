/* Copyright (C) 1997, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu).

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

#include <sysdep.h>
#include "ucontext_i.h"

/* int setcontext(ucontext_t *ctx); */
.weak setcontext
ENTRY(setcontext)

	mov	1, %o1

/* int __setcontext(ucontext_t *ctx, int restoremask); */
ENTRY(__setcontext)

	ldx	[%o0 + UC_SIGMASK], %o2
	stx	%o2, [%o0 + __UC_SIGMASK]
	ta	0x6f

END(__setcontext)
