/* Copyright (C) 1991, 1992, 1997 Free Software Foundation, Inc.
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

#ifdef	 SYS_sethostid

SYSCALL (sethostid, 1)
	ret

#else

/* <bits/errno.h> only defines E* #ifdef _ERRNO_H.  */
#define	_ERRNO_H
#include <bits/errno.h>

ENTRY (sethostid)
	mov ENOSYS, %o0
	sethi %hi(_errno), %g1
	st %o0, [%g1 + %lo(_errno)]
	retl
	sub %g0, 1, %o0

#ifdef	HAVE_GNU_LD

.stabs "warning: sethostid is not implemented and will always fail",30,0,0,0
.stabs "_sethostid",1,0,0,0

#endif

#endif
