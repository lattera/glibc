/* Prepare arguments for library initialization function.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#include <sysdep.h>

#define __S1(x) #x
#define __S(x) __S1(x)

#ifdef PIC

#define SYSDEP_CALL_INIT(NAME, INIT) asm("\
	.weak _dl_starting_up
	.global " #NAME "
	.type " #NAME ",@function
" #NAME ":
	save	%sp, -64, %sp
1:	call	11f
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
11:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
	add	%l7, %o7, %l7
	/* Are we a dynamic libc being loaded into a static program?  */
	sethi	%hi(_dl_starting_up), %l2
	or	%l2, %lo(_dl_starting_up), %l2
	ld	[%l7+%l2], %l2
	cmp	%l2, 0
	beq	3f
	 sethi	%hi(__libc_multiple_libcs), %l3
	ld	[%l2], %l2
	subcc	%g0, %l2, %g0
	subx	%g0, -1, %l2
3:	or	%l3, %lo(__libc_multiple_libcs), %l3
	ld	[%l7+%l3], %l3
	cmp	%l2, 0
	st	%l2, [%l3]
	/* If so, argc et al are in %o0-%o2 already.  Otherwise, load them.  */
	bnz	" #INIT "
	 restore
	ld	[%sp+22*4], %o0
	add	%sp, 23*4, %o1
	sll	%o0, 2, %o2
	add	%o2, %o1, %o2
	ba	" #INIT "
	 add	%o2, 4, %o2
	.size "#NAME ", .-" #NAME);

#else

#define SYSDEP_CALL_INIT(NAME, INIT) asm("\
	.weak _dl_starting_up
	.global " #NAME "
	.type " #NAME ",@function
" #NAME ":
	/* Are we a dynamic libc being loaded into a static program?  */
	sethi	%hi(_dl_starting_up), %g2
	or	%g2, %lo(_dl_starting_up), %g2
	cmp	%g2, 0
	beq	3f
	 sethi	%hi(__libc_multiple_libcs), %g3
	ld	[%g2], %g2
	subcc	%g0, %g2, %g0
	subx	%g0, -1, %g2
3:	or	%g3, %lo(__libc_multiple_libcs), %g3
	cmp	%g2, 0
	st	%g2, [%g3]
	/* If so, argc et al are in %o0-%o2 already.  Otherwise, load them.  */
	bnz	" #INIT "
	 nop
	ld	[%sp+22*4], %o0
	add	%sp, 23*4, %o1
	sll	%o0, 2, %o2
	add	%o2, %o1, %o2
	ba	" #INIT "
	 add	%o2, 4, %o2
	.size "#NAME ", .-" #NAME);

#endif
