 # Optimized strcmp implementation for PowerPC.
 # Copyright (C) 1997 Free Software Foundation, Inc.
 # This file is part of the GNU C Library.
 #
 # The GNU C Library is free software; you can redistribute it and/or
 # modify it under the terms of the GNU Library General Public License as
 # published by the Free Software Foundation; either version 2 of the
 # License, or (at your option) any later version.
 #
 # The GNU C Library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Library General Public License for more details.
 #
 # You should have received a copy of the GNU Library General Public
 # License along with the GNU C Library; see the file COPYING.LIB.  If not,
 # write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 # Boston, MA 02111-1307, USA.

 # See strlen.s for comments on how the end-of-string testing works.

	.section ".text"
	.align 3
	.globl strcmp
	.type strcmp,@function
strcmp:
 # int [r3] strcmp (const char *p1 [r3], const char *p2 [r4])

 # General register assignments:
 # r0:	temporary
 # r3:	pointer to previous word in s1
 # r4:	pointer to previous word in s2
 # r5:	current first word in s1
 # r6:	current first word in s2 (after re-alignment)
 # r7:	0xfefefeff
 # r8:	0x7f7f7f7f
 # r9:	~(word in s1 | 0x7f7f7f7f)
	
 # Register assignments in the prologue:
 # r10:	low 2 bits of p2-p1
 # r11:	mask to orc with r5/r6
	
	subf. %r10,%r4,%r3
	beq-  equal
	andi. %r10,%r10,3
	cmpi  %cr1,%r10,2
	beq-  %cr1,align2
	lis   %r7,0xfeff
	lis   %r8,0x7f7f
	addi  %r8,%r8,0x7f7f
	addi  %r7,%r7,0xfffffeff
	bgt-  %cr1,align3
strcmp3:
	rlwinm %r0,%r3,3,27,28
	li    %r11,-1
	srw   %r11,%r11,%r0
	clrrwi %r3,%r3,2
	clrrwi %r4,%r4,2
	lwz   %r5,0(%r3)
	lwz   %r6,0(%r4)
	bne-  align1

 # The loop, case when both strings are aligned the same.
 # on entry, cr1.eq must be 1.
 # r10:	second word in s1
 # r11:	second word in s2 OR mask to orc with first two words.
align0:	
	andi. %r0,%r3,4
	orc   %r5,%r5,%r11
	orc   %r6,%r6,%r11
	beq+  a0start
	add   %r0,%r7,%r5
	nor   %r9,%r8,%r5
	and.  %r0,%r0,%r9
	cmplw %cr1,%r5,%r6
	subi  %r3,%r3,4
	bne-  endstringeq
	subi  %r4,%r4,4
	bne-  %cr1,difference

loopalign0:
	lwzu  %r5,8(%r3)
	bne-  %cr1,difference2
	lwzu  %r6,8(%r4)
a0start:
	add   %r0,%r7,%r5
	nor   %r9,%r8,%r5
	and.  %r0,%r0,%r9
	cmplw %cr1,%r5,%r6
	lwz   %r10,4(%r3)
	bne-  endstringeq
	add   %r0,%r7,%r10
	bne-  %cr1,difference
	nor   %r9,%r8,%r10
	lwz   %r11,4(%r4)
	and.  %r0,%r0,%r9
	cmplw %cr1,%r10,%r11
	beq+  loopalign0

	mr    %r5,%r10
	mr    %r6,%r11

 # fall through to...

endstringeq:
 # (like 'endstring', but an equality code is in cr1)
	beq  %cr1,equal
endstring:
 # OK. We've hit the end of the string. We need to be careful that
 # we don't compare two strings as different because of gunk beyond
 # the end of the strings. We do it like this...
	and  %r0,%r8,%r5
	add  %r0,%r0,%r8
	xor. %r10,%r5,%r6
	andc %r9,%r9,%r0
	cntlzw %r10,%r10
	cntlzw %r9,%r9
	addi %r9,%r9,7
	cmpw %cr1,%r9,%r10
	blt  %cr1,equal
	sub  %r3,%r5,%r6
	bgelr+
	mr   %r3,%r6
	blr
equal:	li   %r3,0
	blr
	
 # The loop, case when s2 is aligned 1 char behind s1.
 # r10:	current word in s2 (before re-alignment)

align1:
	cmpwi %cr1,%r0,0
	orc   %r5,%r5,%r11
	bne   %cr1,align1_123
 # When s1 is aligned to a word boundary, the startup processing is special.
	slwi. %r6,%r6,24
	bne+  a1entry_0
	nor   %r9,%r8,%r5
	b     endstring

align1_123:
 # Otherwise (s1 not aligned to a word boundary):
	mr    %r10,%r6
	add   %r0,%r7,%r5
	nor   %r9,%r8,%r5
	and.  %r0,%r0,%r9
	srwi  %r6,%r6,8
	orc   %r6,%r6,%r11
	cmplw %cr1,%r5,%r6
	bne-  endstringeq
	bne-  %cr1,difference

loopalign1:
	slwi. %r6,%r10,24
	bne-  %cr1,a1difference
	lwzu  %r5,4(%r3)
	beq-  endstring1
a1entry_0:
	lwzu  %r10,4(%r4)
a1entry_123:	
	add   %r0,%r7,%r5
	nor   %r9,%r8,%r5
	and.  %r0,%r0,%r9
	rlwimi %r6,%r10,24,8,31
	cmplw %cr1,%r5,%r6
	beq+  loopalign1
	b     endstringeq

endstring1:
	srwi  %r3,%r5,24
	blr

a1difference:
	lbz   %r6,-1(%r4)
	slwi  %r6,%r6,24
	rlwimi %r6,%r10,24,8,31

 # fall through to...
		
difference:	
 # The idea here is that we could just return '%r5 - %r6', except
 # that the result might overflow. Overflow can only happen when %r5
 # and %r6 have different signs (thus the xor), in which case we want to
 # return negative iff %r6 has its high bit set so %r5 < %r6.
 # A branch-free implementation of this is
 #	xor  %r0,%r5,%r6
 #	rlwinm %r0,%r0,1,31,31
 #	rlwnm %r5,%r5,%r0,1,31
 #	rlwnm %r6,%r6,%r0,1,31
 #	sub  %r3,%r5,%r6
 #	blr
 # but this is usually more expensive.
	xor. %r0,%r5,%r6
	sub  %r3,%r5,%r6
	bgelr+
	mr   %r3,%r6
	blr

difference2:
 # As for 'difference', but use registers r10 and r11 instead of r5 and r6.
	xor. %r0,%r10,%r11
	sub  %r3,%r10,%r11
	bgelr+
	mr   %r3,%r11
	blr
	
 # For the case when s2 is aligned 3 chars behind s1, we switch
 # s1 and s2...
 # r10:	used by 'align2' (see below)
 # r11:	used by 'align2' (see below)
 # r12:	saved link register
 # cr0.eq: must be left as 1.

align3:	mflr %r12
	mr   %r0,%r3
	mr   %r3,%r4
	mr   %r4,%r0
	bl   strcmp3
	mtlr %r12
	neg  %r3,%r3
	blr
	
 # The loop, case when s2 and s1's alignments differ by 2
 # This is the ugly case...
 # FIXME: on a 601, the loop takes 7 cycles instead of the 6 you'd expect,
 # because there are too many branches. This loop should probably be
 # coded like the align1 case.
	
a2even:	lhz   %r5,0(%r3)
	lhz   %r6,0(%r4)
	b     a2entry
	
align2:
	andi. %r0,%r3,1
	beq+  a2even
	subi  %r3,%r3,1
	subi  %r4,%r4,1
	lbz   %r5,1(%r3)
	lbz   %r6,1(%r4)
	cmpwi %cr0,%r5,0
	cmpw  %cr1,%r5,%r6
	beq-  align2end2
	lhzu  %r5,2(%r3)
	beq+  %cr1,a2entry1
	lbz   %r5,-1(%r3)
	sub   %r3,%r5,%r6
	blr

loopalign2:
	cmpw  %cr1,%r5,%r6
	beq-  align2end2
	lhzu  %r5,2(%r3)
	bne-  %cr1,align2different
a2entry1:
	lhzu  %r6,2(%r4)
a2entry:	
	cmpwi %cr5,%r5,0x00ff
	andi. %r0,%r5,0x00ff
	bgt+  %cr5,loopalign2

align2end:
	andi. %r3,%r6,0xff00
	neg   %r3,%r3
	blr

align2different:
	lhzu  %r5,-2(%r3)
align2end2:
	sub   %r3,%r5,%r6
	blr
		
0:
	.size	 strcmp,0b-strcmp
