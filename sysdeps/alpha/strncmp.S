/* Copyright (C) 1996, 1997, 2003 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu)
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

/* Bytewise compare two null-terminated strings of length no longer than N.  */

#include <sysdep.h>

	.set noat
	.set noreorder

	.text

ENTRY(strncmp)
#ifdef PROF
	ldgp	gp, 0(pv)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

	xor	a0, a1, t2	# e0    : are s1 and s2 co-aligned?
	beq	a2, $zerolength	# .. e1 :
	ldq_u	t0, 0(a0)	# e0    : give cache time to catch up
	ldq_u	t1, 0(a1)	# .. e1 :
	and	t2, 7, t2	# e0    :
	and	a0, 7, t4	# .. e1 : find s1 misalignment
	lda	t3, -1		# e0    :
	addq	a2, t4, a2	# .. e1 : bias count by s1 misalignment
	and	a2, 7, t10	# e1    : ofs of last byte in last word
	srl	a2, 3, a2	# .. e0 : remaining full words in count
	and	a1, 7, t5	# e0    : find s2 misalignment
	bne	t2, $unaligned	# .. e1 :

	/* On entry to this basic block:
	   t0 == the first word of s1.
	   t1 == the first word of s2.
	   t3 == -1.  */

$aligned:
	mskqh	t3, a1, t3	# e0    : mask off leading garbage
	nop			# .. e1 :
	ornot	t1, t3, t1	# e0    :
	ornot	t0, t3, t0	# .. e1 :
	cmpbge	zero, t1, t7	# e0    : bits set iff null found
	beq	a2, $eoc	# .. e1 : check end of count
	subq	a2, 1, a2	# e0    :
	bne	t7, $eos	# .. e1 :

	/* Aligned compare main loop.
	   On entry to this basic block:
	   t0 == an s1 word.
	   t1 == an s2 word not containing a null.  */

$a_loop:
	xor	t0, t1, t2	# e0	:
	bne	t2, $wordcmp	# .. e1 (zdb)
	ldq_u	t1, 8(a1)	# e0    :
	ldq_u	t0, 8(a0)	# .. e1 :
	addq	a1, 8, a1	# e0    :
	addq	a0, 8, a0	# .. e1 :
	cmpbge	zero, t1, t7	# e0    :
	beq	a2, $eoc	# .. e1 :
	subq	a2, 1, a2	# e0    :
	beq	t7, $a_loop	# .. e1 :
	br	$eos		# e1    :

	/* The two strings are not co-aligned.  Align s1 and cope.  */
$unaligned:
	subq	a1, t4, a1	# e0	 :
	unop			#        :

	/* If s2 misalignment is larger than s2 misalignment, we need
	   extra startup checks to avoid SEGV.  */

	cmplt	t4, t5, t8	# .. e1 :
	beq	t8, $u_head	# e1    :

	mskqh	t3, t5, t3	# e0    :
	ornot	t1, t3, t3	# e0    :
	cmpbge	zero, t3, t7	# e1    : is there a zero?
	beq	t7, $u_head	# e1    :

	/* We've found a zero in the first partial word of s2.  Align
	   our current s1 and s2 words and compare what we've got.  */

	extql	t1, t5, t1	# e0    :
	lda	t3, -1		# .. e1 :
	insql	t1, a0, t1	# e0    :
	mskqh	t3, a0, t3	# e0    :
	ornot	t1, t3, t1	# e0    :
	ornot	t0, t3, t0	# .. e1 :
	cmpbge	zero, t1, t7	# e0    : find that zero again
	beq	a2, $eoc	# .. e1 : and finish up
	br	$eos		# e1    :

	.align 3
$u_head:
	/* We know just enough now to be able to assemble the first
	   full word of s2.  We can still find a zero at the end of it.

	   On entry to this basic block:
	   t0 == first word of s1
	   t1 == first partial word of s2.  */

	ldq_u	t2, 8(a1)	# e0    : load second partial s2 word
	lda	t3, -1		# .. e1 : create leading garbage mask
	extql	t1, a1, t1	# e0    : create first s2 word
	mskqh	t3, a0, t3	# e0    :
	extqh	t2, a1, t4	# e0    :
	ornot	t0, t3, t0	# .. e1 : kill s1 garbage
	or	t1, t4, t1	# e0    : s2 word now complete
	ornot	t1, t3, t1	# e1    : kill s2 garbage
	cmpbge	zero, t0, t7	# e0    : find zero in first s1 word
	beq	a2, $eoc	# .. e1 :
	lda	t3, -1		# e0    :
	bne	t7, $eos	# .. e1 :
	subq	a2, 1, a2	# e0    :
	xor	t0, t1, t4	# .. e1 : compare aligned words
	mskql	t3, a1, t3	# e0    : mask out s2[1] bits we have seen
	bne	t4, $wordcmp	# .. e1 :
	or	t2, t3, t3	# e0    :
	cmpbge	zero, t3, t7	# e1    : find zero in high bits of s2[1]
	bne	t7, $u_final	# e1    :

	/* Unaligned copy main loop.  In order to avoid reading too much,
	   the loop is structured to detect zeros in aligned words from s2.
	   This has, unfortunately, effectively pulled half of a loop
	   iteration out into the head and half into the tail, but it does
	   prevent nastiness from accumulating in the very thing we want
	   to run as fast as possible.

	   On entry to this basic block:
	   t2 == the unshifted low-bits from the next s2 word.  */

	.align 3
$u_loop:
	extql	t2, a1, t3	# e0    :
	ldq_u	t2, 16(a1)	# .. e1 : load next s2 high bits
	ldq_u	t0, 8(a0)	# e0    : load next s1 word
	addq	a1, 8, a1	# .. e1 :
	addq	a0, 8, a0	# e0    :
	nop			# .. e1 :
	extqh	t2, a1, t1	# e0    :
	cmpbge	zero, t0, t7	# .. e1 : find zero in current s1 word
	or	t1, t3, t1	# e0    :
	beq	a2, $eoc	# .. e1 : check for end of count
	subq	a2, 1, a2	# e0    :
	bne	t7, $eos	# .. e1 :
	xor	t0, t1, t4	# e0    : compare the words
	bne	t4, $wordcmp	# .. e1 (zdb)
	cmpbge	zero, t2, t4	# e0    : find zero in next low bits
	beq	t4, $u_loop	# .. e1 (zdb)

	/* We've found a zero in the low bits of the last s2 word.  Get
	   the next s1 word and align them.  */
$u_final:
	ldq_u	t0, 8(a0)	# e1    :
	extql	t2, a1, t1	# .. e0 :
	cmpbge	zero, t1, t7	# e0    :
	bne	a2, $eos	# .. e1 :

	/* We've hit end of count.  Zero everything after the count
	   and compare whats left.  */

	.align 3
$eoc:
	mskql	t0, t10, t0
	mskql	t1, t10, t1

	/* We've found a zero somewhere in a word we just read.
	   On entry to this basic block:
	   t0 == s1 word
	   t1 == s2 word
	   t7 == cmpbge mask containing the zero.  */

$eos:
	negq	t7, t6		# e0    : create bytemask of valid data
	and	t6, t7, t8	# e1    :
	subq	t8, 1, t6	# e0    :
	or	t6, t8, t7	# e1    :
	zapnot	t0, t7, t0	# e0    : kill the garbage
	zapnot	t1, t7, t1	# .. e1 :
	xor	t0, t1, v0	# e0    : and compare
	beq	v0, $done	# .. e1 :

	/* Here we have two differing co-aligned words in t0 & t1.
	   Bytewise compare them and return (t0 > t1 ? 1 : -1).  */
$wordcmp:
	cmpbge	t0, t1, t2	# e0    : comparison yields bit mask of ge
	cmpbge	t1, t0, t3	# .. e1 :
	xor	t2, t3, t0	# e0    : bits set iff t0/t1 bytes differ
	negq	t0, t1		# e1    : clear all but least bit
	and	t0, t1, t0	# e0    :
	lda	v0, -1		# .. e1 :
	and	t0, t2, t1	# e0    : was bit set in t0 > t1?
	cmovne	t1, 1, v0	# .. e1 (zdb)

$done:
	ret			# e1    :

$zerolength:
	clr	v0
	ret

	END(strncmp)
libc_hidden_builtin_def (strncmp)
