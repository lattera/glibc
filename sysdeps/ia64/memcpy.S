/* Optimized version of the standard memcpy() function.
   This file is part of the GNU C Library.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Dan Pop <Dan.Pop@cern.ch>.

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

/* Return: dest

   Inputs:
        in0:    dest
        in1:    src
        in2:    byte count

   An assembly implementation of the algorithm used by the generic C
   version from glibc.  The case when all three arguments are multiples
   of 8 is treated separatedly, for extra performance.

   In this form, it assumes little endian mode.  For big endian mode,
   sh1 must be computed using an extra instruction: sub sh1 = 64, sh1
   and the order of r[MEMLAT] and r[MEMLAT+1] must be reverted in the
   shrp instruction.  */

#include <sysdep.h>
#undef ret

#define OP_T_THRES 	16
#define OPSIZ 		8

#define saved_pfs	r14
#define sf		r15
#define rescnt		r16
#define saved_pr	r17
#define saved_lc	r18
#define dest		r19
#define src		r20
#define len		r21
#define asrc		r22
#define tmp2		r23
#define tmp3		r24
#define	tmp4		r25
#define ptable		r26
#define ploop56		r27
#define	loopaddr	r28
#define	sh1		r29
#define loopcnt		r30
#define	value		r31

#define dl0		r22
#define dh0		r23
#define dl1		r24
#define dh1		r25
#define dl2		r26
#define dh2		r27
#define dl3		r28
#define dh3		r29 

#define LOOP(shift)							\
		.align	32 ; 						\
.loop##shift##:								\
(p[0])		ld8	r[0] = [asrc], 8 ;	/* w1 */		\
(p[MEMLAT+1])	st8	[dest] = value, 8 ;				\
(p[MEMLAT])	shrp	value = r[MEMLAT], r[MEMLAT+1], shift ;	\
		nop.b	0 ;						\
		nop.b	0 ;						\
		br.ctop.sptk .loop##shift ;				\
		br.cond.sptk .cpyfew ; /* deal with the remaining bytes */

ENTRY(memcpy)
	alloc 	saved_pfs = ar.pfs, 3, 40-3, 0, 40
#include "softpipe.h"
	.rotr	r[MEMLAT + 2], q[MEMLAT + 1], s0[2], s1[2], s2[2], s3[2]
	.rotf	tl0[5], th0[5], tl1[5], th1[5], tl2[5], th2[5], tl3[5], th3[5]
	.rotp	p[MEMLAT + 2]
	mov	ret0 = in0		// return value = dest
	mov	saved_pr = pr		// save the predicate registers
// brp is currently broken - reenable when it gets fixed.
//	brp.loop.many	.l0, .done - 16
        mov 	saved_lc = ar.lc	// save the loop counter
	or	tmp3 = in0, in1 ;;	// tmp3 = dest | src
	or	tmp3 = tmp3, in2	// tmp3 = dest | src | len
	mov 	dest = in0		// dest
	mov 	src = in1		// src
	mov	len = in2		// len
	sub	tmp2 = r0, in0		// tmp2 = -dest
	cmp.eq	p6, p0 = in2, r0	// if (len == 0)
(p6)	br.cond.spnt .restore_and_exit;;// 	return dest;
	and	tmp4 = 7, tmp3 		// tmp4 = (dest | src | len) & 7
	tbit.nz	p8, p0 = src, 3 ;;	// test for 16-byte boundary align
	cmp.ne	p6, p0 = tmp4, r0	// if ((dest | src | len) & 7 != 0)
(p6)	br.cond.sptk .next		//	goto next;

// The optimal case, when dest, src and len are all multiples of 8

(p8)	ld8	value = [src], 8	// align src if necessary
(p8)	adds	len = -8, len ;;	// adjust len accordingly
	shr.u	loopcnt = len, 6 	// loopcnt = len / 64
	shr.u	rescnt = len, 3		// rescnt = len / 8
	mov	pr.rot = 1 << 16 	// set rotating predicates
	mov	ar.ec = 4 + 1 ;;	// set the epilog counter
	cmp.eq	p6, p0 = loopcnt, r0 
	and	rescnt = 7, rescnt	// resnt = residual word count
	adds	loopcnt = -1, loopcnt	// --loopcnt
(p8)	st8	[dest] = value, 8	// copy one word if aligning 
(p6)	br.cond.spnt .epilog;;		// there are < 8 words to copy
	add	sf = 64 * 4, src
	mov	ar.lc = loopcnt 	// set the loop counter		 
	mov	s0[1] = src
	add	s1[1] = 16*1, src
	add     s2[1] = 16*2, src
	add	s3[1] = 16*3, src
	;;
	mov     dl0 = dest
	add	dh0 = 8 * 1, dest
	add	dl1 = 8 * 2, dest
	add     dh1 = 8 * 3, dest
	add	dl2 = 8 * 4, dest
	add	dh2 = 8 * 5, dest
	add	dl3 = 8 * 6, dest
	add	dh3 = 8 * 7, dest
	;;	
.l0:
(p[0]) 	lfetch.nta [sf], 64

(p[0])  ldfp8   tl0[0], th0[0] = [s0[1]]
(p[0])  ldfp8   tl1[0], th1[0] = [s1[1]]
(p[0])  ldfp8   tl2[0], th2[0] = [s2[1]]
(p[0])  ldfp8   tl3[0], th3[0] = [s3[1]]

(p[0])  add     s0[0] = 64, s0[1]
(p[0])  add     s1[0] = 64, s1[1]
(p[0])  add     s2[0] = 64, s2[1]
(p[0])  add     s3[0] = 64, s3[1]
(p[1])	mov	src = s0[1]		// for the epilog code

(p[4])  stf8    [dl0] = tl0[4], 64
(p[4])  stf8    [dh0] = th0[4], 64
(p[4])  stf8    [dl1] = tl1[4], 64
(p[4])  stf8    [dh1] = th1[4], 64
(p[4])  stf8    [dl2] = tl2[4], 64
(p[4])  stf8    [dh2] = th2[4], 64
(p[4])  stf8    [dl3] = tl3[4], 64
(p[4])  stf8    [dh3] = th3[4], 64

	br.ctop.sptk.many .l0
.done:
	mov	dest = dl0
.epilog:
	cmp.eq	p6, p0 = rescnt, r0	// are there any words left to copy?
	tbit.nz	p10, p0 = rescnt, 0
(p6)	br.cond.spnt .restore_and_exit ;;
(p10)	ld8	r[0] = [src], 8
	tbit.nz	p11, p0 = rescnt, 1 ;;
(p11)	ld8	r[1] = [src], 8
(p10)	st8	[dest] = r[0], 8 ;;
(p11)	ld8	r[2] = [src], 8 
(p11)	st8	[dest] = r[1], 8
	tbit.nz	p12, p0 = rescnt, 2 ;;
(p12)	ld8	r[3] = [src], 8
(p11)	st8	[dest] = r[2], 8 ;;
(p12)	ld8	r[4] = [src], 8
(p12)	st8	[dest] = r[3], 8 ;;
(p12)	ld8	r[5] = [src], 8
(p12) 	st8	[dest] = r[4], 8 
	mov	ar.lc = saved_lc ;;	// restore the loop counter
(p12) 	ld8	r[6] = [src], 8
(p12)	st8	[dest] = r[5], 8 
	mov	ar.pfs = saved_pfs;;	// restore the PFS
(p12)	st8	[dest] = r[6]
	mov	pr = saved_pr, -1 	// restore the predicate registers
	br.ret.sptk.many b0
.next:
	cmp.ge	p6, p0 = OP_T_THRES, len	// is len <= OP_T_THRES
	and	loopcnt = 7, tmp2 		// loopcnt = -dest % 8
(p6)	br.cond.spnt	.cpyfew			// copy byte by byte
	;;
	cmp.eq	p6, p0 = loopcnt, r0
(p6)	br.cond.sptk	.dest_aligned
	sub	len = len, loopcnt	// len -= -dest % 8
	adds	loopcnt = -1, loopcnt	// --loopcnt
	;;
	mov	ar.lc = loopcnt
.l1:					// copy -dest % 8 bytes
	ld1	value = [src], 1	// value = *src++
	;;
	st1	[dest] = value, 1	// *dest++ = value  
	br.cloop.dptk .l1	
.dest_aligned:
	and	sh1 = 7, src 		// sh1 = src % 8
	and	tmp2 = -8, len   	// tmp2 = len & -OPSIZ
	and	asrc = -8, src		// asrc = src & -OPSIZ  -- align src
	shr.u	loopcnt = len, 3	// loopcnt = len / 8
	and	len = 7, len;;		// len = len % 8
	adds	loopcnt = -1, loopcnt	// --loopcnt
	addl	tmp4 = @ltoff(.table), gp 
	addl	tmp3 = @ltoff(.loop56), gp
	mov     ar.ec = MEMLAT + 1	// set EC
	mov     pr.rot = 1 << 16;;	// set rotating predicates
	mov	ar.lc = loopcnt		// set LC
	cmp.eq  p6, p0 = sh1, r0 	// is the src aligned?
(p6)    br.cond.sptk .src_aligned
	add	src = src, tmp2		// src += len & -OPSIZ
	shl	sh1 = sh1, 3		// sh1 = 8 * (src % 8)
	ld8	ploop56 = [tmp3]	// ploop56 = &loop56
	ld8	ptable = [tmp4];;	// ptable = &table
	add	tmp3 = ptable, sh1;;	// tmp3 = &table + sh1
	mov	ar.ec = MEMLAT + 1 + 1 // one more pass needed
	ld8	tmp4 = [tmp3];;		// tmp4 = loop offset
	sub	loopaddr = ploop56,tmp4	// loopadd = &loop56 - loop offset
	ld8	r[1] = [asrc], 8;;	// w0
	mov	b6 = loopaddr;;
	br	b6			// jump to the appropriate loop

	LOOP(8)
	LOOP(16)
	LOOP(24)
	LOOP(32)
	LOOP(40)
	LOOP(48)
	LOOP(56)
	
.src_aligned:
.l3:
(p[0])		ld8	r[0] = [src], 8
(p[MEMLAT])	st8	[dest] = r[MEMLAT], 8
		br.ctop.dptk .l3
.cpyfew:
	cmp.eq	p6, p0 = len, r0	// is len == 0 ?
	adds	len = -1, len		// --len;
(p6)	br.cond.spnt	.restore_and_exit ;;
	mov	ar.lc = len
.l4:
	ld1	value = [src], 1
	;;
	st1	[dest] = value, 1
	br.cloop.dptk	.l4 ;;
.restore_and_exit:
	mov 	ar.pfs = saved_pfs	// restore the PFS
	mov     pr = saved_pr, -1    	// restore the predicate registers
	mov 	ar.lc = saved_lc	// restore the loop counter
	br.ret.sptk.many b0
	.align 8
.table:
	data8	0			// dummy entry
	data8 	.loop56 - .loop8
	data8 	.loop56 - .loop16
	data8 	.loop56 - .loop24
	data8	.loop56 - .loop32
	data8	.loop56 - .loop40
	data8	.loop56 - .loop48
	data8	.loop56 - .loop56

END(memcpy)
