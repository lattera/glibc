/* PLT trampolines.  m68k version.
   Copyright (C) 2005, 2011 Free Software Foundation, Inc.
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
#ifdef __mcoldfire__
	move.l %d0,-(%sp)
	rts
#else
	jmp (%d0)
#endif
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
#ifdef __mcoldfire__
	move.l %d0,-(%sp)
	rts
#else
	jmp (%d0)
#endif

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
	| Round framesize up to longword alignment
	addq.l #3, %d1
	and.l #-3, %d1
	sub.l %d1, %a0
	move.l %a0, %sp
#ifdef __mcoldfire__
	tst.l %d1
	beq 2f
1:	move.l (%a0)+, (%a1)+
	subq.l #4,%d1
	bne 1b
2:
#else
	lsr.l #2,%d1
	jra 2f
1:	move.l (%a1)+, (%a0)+
2:	dbra %d1,1b
#endif
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
#ifdef __mcoldfire__
	pea 2f(%pc)
	move.l %d0,-(%sp)
	rts
2:
#else
	jsr (%d0)
#endif
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
#ifdef __mcoldfire__
	fmove.d %fp0, -(%sp)
#else
	fmove.x %fp0, -(%sp)
#endif
	move.l %a0, -(%sp)
	move.l %d1, -(%sp)
	move.l %d0, -(%sp)
	pea (%sp)
#ifdef __mcoldfire__
	pea 24(%sp)
	move.l 40(%sp), -(%sp)
	move.l 40(%sp), -(%sp)
#else
	pea 28(%sp)
	move.l 44(%sp), -(%sp)
	move.l 44(%sp), -(%sp)
#endif
	jbsr _dl_call_pltexit
	lea 16(%sp), %sp
	move.l (%sp)+, %d0
	move.l (%sp)+, %d1
	move.l (%sp)+, %a0
#ifdef __mcoldfire__
	fmove.d (%sp)+, %fp0
#else
	fmove.x (%sp)+, %fp0
#endif
	lea 20(%sp), %sp
	rts
	.size _dl_runtime_profile, . - _dl_runtime_profile
