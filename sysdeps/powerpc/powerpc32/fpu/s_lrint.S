/* Round double to long int.  PowerPC32 version.
   Copyright (C) 2004 Free Software Foundation, Inc.
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

/* long int[r3] __lrint (double x[fp1])  */
ENTRY (__lrint)	
	fctiw	fp13,fp1
	stfd	fp13,-8(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	lwz	r3,-4(r1)	
	blr
	END (__lrint)

weak_alias (__lrint, lrint)

strong_alias (__lrint, __lrintf)
weak_alias (__lrint, lrintf)

#ifdef NO_LONG_DOUBLE
strong_alias (__lrint, __lrintl)
weak_alias (__lrint, lrintl)
#endif
