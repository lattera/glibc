/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@azstarnet.com).

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

#include <sysdep.h>

/*
 * This is for COMPATIBILITY with Linux/x86 only.  Linux/Alpha system
 * calls return an error indication in a3.  This allows to return
 * arbitrary 64bit values in v0 (because negative values are not
 * mistaken as error numbers).  However, C allows to return only one
 * value so the interface below folds the error indication passed in
 * a3 back into v0: it sets v0 to -errno if an error occurs.  Thus,
 * no negative 64bit numbers can be returned.  To avoid this problem,
 * use assembly stubs wherever possible/convenient.
 *
 * Usage:
 *
 * long	syscall(syscall_number, arg1, arg2, arg3, arg4, arg5)
 *
 * syscall_number = the index of the system call we're invoking
 * arg1-arg5 = up to 5 integer arguments to the system call
 *
 * We need to do some arg shifting: the kernel expects the
 * syscall number in v0 and the first five args in a0-a4.
 *
 */


LEAF(__syscall, 0)
	bis	a0, a0, v0	# Syscall number -> v0
	bis	a1, a1, a0	# arg1-arg5 -> a0-a4
	bis	a2, a2, a1
	bis	a3, a3, a2
	bis	a4, a4, a3
	bis	a5, a5, a4

	call_pal PAL_callsys	# Invoke system call
	bne	a3, error
	ret

error:	br	gp, 2f
2:	ldgp	gp, 0(gp)
	jmp	zero, syscall_error

weak_alias(__syscall, syscall)
