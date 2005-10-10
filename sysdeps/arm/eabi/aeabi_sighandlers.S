/* Link-time constants for ARM EABI - signal handlers.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

/* The ARM EABI defines these as "functions".  */

#include <sysdep.h>

	.global __aeabi_SIG_DFL
	.hidden __aeabi_SIG_DFL
	.type __aeabi_SIG_DFL, %function
	.set __aeabi_SIG_DFL, 0

	.global __aeabi_SIG_IGN
	.hidden __aeabi_SIG_IGN
	.type __aeabi_SIG_IGN, %function
	.set __aeabi_SIG_IGN, 1

	.global __aeabi_SIG_ERR
	.hidden __aeabi_SIG_ERR
	.type __aeabi_SIG_ERR, %function
	.set __aeabi_SIG_ERR, -1
