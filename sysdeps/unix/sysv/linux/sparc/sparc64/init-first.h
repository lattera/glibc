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
	save	%sp, -192, %sp
1:	call	11f
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
11:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
	add	%l7, %o7, %l7
	/* Are we a dynamic libc being loaded into a static program?  */
	sethi	%hi(_dl_starting_up), %l2
	or	%l2, %lo(_dl_starting_up), %l2
	ldx	[%l7+%l2], %l2
	brz,pn	%l2, 3f
	 sethi	%hi(__libc_multiple_libcs), %l3
	ld	[%l2], %l4
	mov	%g0, %l2
	movrz	%l4, 1, %l2
3:	or	%l3, %lo(__libc_multiple_libcs), %l3
	ldx	[%l7+%l3], %l3
	st	%l2, [%l3]
	/* If so, argc et al are in %o0-%o2 already.  Otherwise, load them.  */
	brnz,pn	%l2, " #INIT "
	 restore
	ldx	[%sp+" __S(STACK_BIAS) "+22*8], %o0
	add	%sp, " __S(STACK_BIAS) "+23*8, %o1
	sllx	%o0, 3, %o2
	add	%o2, %o1, %o2
	ba	" #INIT "
	 add	%o2, 8, %o2
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
	brz,pt	%g2, 3f
	 sethi	%hi(__libc_multiple_libcs), %g3
	ld	[%g2], %g1
	mov	%g0, %g2
	movrz	%g1, 1, %g2
3:	st	%g2, [%g3 + %lo(__libc_multiple_libcs)]
	/* If so, argc et al are in %o0-%o2 already.  Otherwise, load them.  */
	brnz,pn	%g2, " #INIT "
	 nop
	ldx	[%sp+" __S(STACK_BIAS) "+22*8], %o0
	add	%sp, " __S(STACK_BIAS) "+23*8, %o1
	sllx	%o0, 3, %o2
	add	%o2, %o1, %o2
	ba	" #INIT "
	 add	%o2, 8, %o2
	.size "#NAME ", .-" #NAME);

#endif
