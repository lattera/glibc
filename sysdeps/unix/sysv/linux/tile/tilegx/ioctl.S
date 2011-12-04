/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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


/* TILE-Gx specifies that "unsigned int" is sign extended in the high
   32 bits.  But since the userspace API claims to be "unsigned long",
   calls into __ioctl() will not be sign extended, but rather pass all
   64 bits of the argument.  Therefore, when we pass the "request"
   value to the kernel, we must explicitly sign-extend it to match the
   kernel's internal use of "unsigned int" as the second argument,
   which we do by casting to "unsigned int".  */

#include <sysdep.h>

	.text
ENTRY (__ioctl)
	FEEDBACK_ENTER(__ioctl)
	{
	 addxi r1, r1, 0
	 moveli TREG_SYSCALL_NR_NAME, __NR_ioctl
	}
	swint1
	BNEZ r1, 0f
	jrp lr
PSEUDO_END (__ioctl)
weak_alias (__ioctl, ioctl)
