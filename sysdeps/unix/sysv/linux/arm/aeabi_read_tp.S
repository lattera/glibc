/* Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <sysdep.h>

#ifdef HAVE_TLS_SUPPORT

/* GCC will emit calls to this routine under -mtp=soft.  Linux has an
   equivalent helper function (which clobbers fewer registers than
   a normal function call) in a high page of memory; tail call to the
   helper.  */

	.hidden __aeabi_read_tp
ENTRY (__aeabi_read_tp)
	mov	r0, #0xffff0fff
	sub	pc, r0, #31
END (__aeabi_read_tp)

#endif
