/* Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@tamu.edu>, 1996.
   EV67 optimized by Rick Gorton <rick.gorton@alpha-processor.com>.

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

/* Append no more than COUNT characters from the null-terminated string SRC
   to the null-terminated string DST.  Always null-terminate the new DST.  */

#include <sysdep.h>

	.arch ev6
	.set noreorder
	.text

ENTRY(strncat)
	ldgp	gp, 0(pv)
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	mov	$16, $0		# set up return value
	beq	$18, $zerocount	# U :
	/* Find the end of the string.  */
	ldq_u   $1, 0($16)	# L : load first quadword ($16 may be misaligned)
	lda     $2, -1($31)	# E :

	insqh   $2, $0, $2	# U :
	andnot  $16, 7, $16	# E :
	nop			# E :
	or      $2, $1, $1	# E :

	nop			# E :
	nop			# E :
	cmpbge  $31, $1, $2	# E : bits set iff byte == 0
	bne     $2, $found	# U :

$loop:	ldq     $1, 8($16)	# L :
	addq    $16, 8, $16	# E :
	cmpbge  $31, $1, $2	# E :
	beq     $2, $loop	# U :

$found:	cttz	$2, $3		# U0 :
	addq	$16, $3, $16	# E :
	jsr	$23, __stxncpy	# L0 :/* Now do the append.  */

	/* Worry about the null termination.  */

	zapnot	$1, $27, $2	# U : was last byte a null?
	cmplt	$27, $24, $5	# E : did we fill the buffer completely?
	bne	$2, 0f		# U :
	ret			# L0 :

0:	or	$5, $18, $2	# E :
	nop
	bne	$2, 2f		# U :
	and	$24, 0x80, $3	# E : no zero next byte

	nop			# E :
	bne	$3, 1f		# U :
	/* Here there are bytes left in the current word.  Clear one.  */
	addq	$24, $24, $24	# E : end-of-count bit <<= 1
	nop			# E :

2:	zap	$1, $24, $1	# U :
	nop			# E :
	stq_u	$1, 0($16)	# L :
	ret			# L0 :

1:	/* Here we must clear the first byte of the next DST word */
	stb	$31, 8($16)	# L :
	nop			# E :
	nop			# E :
	ret			# L0 :

$zerocount:
	nop			# E :
	nop			# E :
	nop			# E :
	ret			# L0 :

	END(strncat)
