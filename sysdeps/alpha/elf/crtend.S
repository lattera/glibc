/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu)

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


/*
 * Tails of the constructor/destructor lists.
 */

/* The __*TOR_END__ symbols are not global because when this file is used
   in a shared library, we do not want the symbol to fall over to the
   application's lists.  */

.section .ctors,"aw"

	.align 3
__CTOR_END__:
	.quad   0

.section .dtors,"aw"

	.align 3
__DTOR_END__:
	.quad   0


/*
 * Fragment of the ELF _init routine that invokes our ctor startup
 */

.section .init,"ax"

	/* Since the bits of the _init function are spread across many
	   object files, each potentially with its own GP, we must
	   assume we need to load ours.  Further, our .init section
	   can easily be more than 4MB away from our .text bits so we
	   can't use bsr.  */

	br      $gp,1f
1:	ldgp    $gp,0($gp)
	jsr     $26,__do_global_ctors_aux

	/* Must match the alignment we got from crti.o else we get
	   zero-filled holes in our _init function and thense SIGILL.  */
	.align 3

/*
 * Invoke our destructors in order.
 */

.text

	.align 3
	.ent __do_global_ctors_aux

__do_global_ctors_aux:
	.frame  $sp,16,$26,0
	/* GP already loaded in .init.  */
	lda     $sp,-16($sp)
	stq     $9,8($sp)
	stq     $26,0($sp)
	.mask   (1<<26)|(1<<9), -16
	.prologue 0

	lda     $9,__CTOR_END__
	br      1f
0:	jsr     $26,($27)
1:	ldq     $27,-8($9)
	subq    $9,8,$9
	not     $27,$0
	bne     $0,0b

	ldq     $26,0($sp)
	ldq     $9,8($sp)
	lda     $sp,16($sp)
	ret

	.end __do_global_ctors_aux
