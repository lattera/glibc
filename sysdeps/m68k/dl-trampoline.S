/* PLT trampolines.  m68k version.
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

#include <sysdep.h>

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
_dl_runtime_resolve:
	| Save %a0 (struct return address) and %a1.
	move.l %a0, -(%sp)
	move.l %a1, -(%sp)
	| Call the real address resolver.
	jbsr _dl_fixup
	| Restore register %a0 and %a1.
	move.l (%sp)+, %a1
	move.l (%sp)+, %a0
	| Pop parameters
	addq.l #8, %sp
	| Call real function.
	jmp (%d0)
	.size _dl_runtime_resolve, . - _dl_runtime_resolve

	.text
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
_dl_runtime_profile:
	pea 8(%sp)
	move.l %a1, -(%sp)
	move.l %a0, -(%sp)
	pea -1.w
	| Push parameters for _dl_profile_fixup
	pea (%sp)
	pea 8(%sp)
	move.l 32(%sp), -(%sp)
	move.l 32(%sp), -(%sp)
	move.l 32(%sp), -(%sp)
	subq.l #8, %sp
	| Call the real address resolver.
	jbsr _dl_profile_fixup
	| Pop parameters
	lea 28(%sp), %sp
	move.l (%sp), %d1
	jpl 1f
	addq.l #4, %sp
	| Restore register %a0 and %a1.
	move.l (%sp)+, %a0
	move.l (%sp)+, %a1
	lea 12(%sp), %sp
	| Call real function.
	jmp (%d0)

	/*
	    +24     return address
	    +20     PLT1
	    +16     PLT2
	    +12     %sp
	    +8      %a1
	    +4      %a0
	   %sp      free
	*/
1:	move.l %a2, (%sp)
	move.l %sp, %a2
	move.l %sp, %a0
	lea 28(%sp), %a1
	| Round framesize up to even
	addq.l #1, %d1
	lsr #1, %d1
	sub.l %d1, %a0
	sub.l %d1, %a0
	move.l %a0, %sp
	jra 2f
1:	move.w (%a1)+, (%a0)+
2:	dbra %d1,1b
	/*
	   %a2+24  return address
	   %a2+20  PLT1
	   %a2+16  PLT2
	   %a2+12  %sp
	   %a2+8   %a1
	   %a2+4   %a0
	   %a2     %a2
	   %sp     copied stack frame
	*/

	move.l 4(%a2), %a0
	move.l 8(%a2), %a1
	jsr (%d0)
	move.l %a2, %sp
	move.l (%sp)+, %a2
	/*
	    +20     return address
	    +16     PLT1
	    +12     PLT2
	    +8      %sp
	    +4      %a1
	   %sp      %a0
	*/
	fmove.x %fp0, -(%sp)
	move.l %d1, -(%sp)
	move.l %d0, -(%sp)
	pea (%sp)
	pea 20(%sp)
	move.l 40(%sp), -(%sp)
	move.l 40(%sp), -(%sp)
	jbsr _dl_call_pltexit
	lea 16(%sp), %sp
	move.l (%sp)+, %d0
	move.l (%sp)+, %d1
	fmove.x (%sp)+, %fp0
	lea 20(%sp), %sp
	rts
	.size _dl_runtime_profile, . - _dl_runtime_profile
