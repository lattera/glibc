/* Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#define _ERRNO_H
#include <errnos.h>

/* Linux has two versions of the select system call.  The old one expected
   one argument which must be a pointer to a struct which contains the
   five values.  The new version expects the five arguments be given in the
   registers.  First try the new version, if it's not available fall back
   to the old version.  */

	.text
	SYSCALL_ERROR_HANDLER
ENTRY (__select)

#if 0 /* For now only use the old version.  */
	DO_CALL (#SYS_ify (_newselect), 5)
	tst.l %d0
	jmi 1f
	rts

1:	move.l #-ENOSYS, %d1
	cmp.l %d1, %d0
	jne syscall_error	/* Real error */

	/* Try again using the old syscall interface.  */
#endif
	lea 4(%sp), %a0
	move.l %a0, %d1
	move.l #SYS_ify (select), %d0
	trap #0
	tst.l %d0
	jmi syscall_error
	ret

weak_alias (__select, select)
