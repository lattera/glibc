/* Optimized version of the standard bzero() function.
   This file is part of the GNU C Library.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Dan Pop <Dan.Pop@cern.ch>.

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

/* Return: dest
  
   Inputs:
        in0:    dest
        in1:    count

   The algorithm is fairly straightforward: set byte by byte until we
   we get to a word aligned address, then set word by word as much as
   possible; the remaining few bytes are set one by one.  */

#include <sysdep.h>
#undef ret

#define dest		in0
#define	cnt		in1

#define save_pfs 	loc0
#define ptr1		loc1
#define ptr2		loc2
#define tmp		loc3
#define	loopcnt		loc4
#define save_lc		loc5

ENTRY(bzero)
	.prologue
	alloc	save_pfs = ar.pfs, 2, 6, 0, 0	
	.save ar.lc, save_lc
	mov	save_lc = ar.lc
	.body
	mov	ret0 = dest
	and	tmp = 7, dest
	cmp.eq	p6, p0 = cnt, r0
(p6)	br.cond.spnt .restore_and_exit ;;
	mov	ptr1 = dest
	sub	loopcnt = 8, tmp
	cmp.gt	p6, p0 = 16, cnt
(p6)	br.cond.spnt .set_few;;
	cmp.eq	p6, p0 = tmp, r0
(p6)	br.cond.sptk .dest_aligned
	sub	cnt = cnt, loopcnt
	adds	loopcnt = -1, loopcnt;;
	mov	ar.lc = loopcnt;;	
.l1:
	st1	[ptr1] = r0, 1
	br.cloop.dptk	.l1 ;;
.dest_aligned:
	adds	ptr2 = 8, ptr1
	shr.u	loopcnt = cnt, 4 ;;	// loopcnt = cnt / 16
	cmp.eq	p6, p0 = loopcnt, r0
(p6)	br.cond.spnt	.one_more
	and	cnt = 0xf, cnt		// compute the remaining cnt
	adds	loopcnt = -1, loopcnt;;
	mov     ar.lc = loopcnt;;	
.l2:
	st8	[ptr1] = r0, 16
	st8	[ptr2] = r0, 16
	br.cloop.dptk .l2
	cmp.le	p6, p0 = 8, cnt	;;
.one_more:
(p6)	st8     [ptr1] = r0, 8
(p6)	adds	cnt = -8, cnt ;;
	cmp.eq	p6, p0 = cnt, r0
(p6)	br.cond.spnt	.restore_and_exit
.set_few:
	adds	loopcnt = -1, cnt;;
	mov	ar.lc = loopcnt;;
.l3:	
	st1     [ptr1] = r0, 1
	br.cloop.dptk   .l3 ;;	
.restore_and_exit:
	mov	ar.lc = save_lc
	mov	ar.pfs = save_pfs
	br.ret.sptk.many b0					
END(bzero)
