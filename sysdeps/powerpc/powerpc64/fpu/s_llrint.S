/* Round double to long int.  PowerPC64 version.
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

/* long long int[r3] __llrint (double x[fp1])  */
ENTRY (__llrint)	
	CALL_MCOUNT 0
	fctid	fp13,fp1
	stfd	fp13,-16(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3,-16(r1)	
	blr
	END (__llrint)

strong_alias (__llrint, __lrint)
weak_alias (__llrint, llrint)
weak_alias (__lrint, lrint)

#ifdef NO_LONG_DOUBLE
strong_alias (__llrint, __llrintl)
weak_alias (__llrint, llrintl)
strong_alias (__lrint, __lrintl)
weak_alias (__lrint, lrintl)
#endif
