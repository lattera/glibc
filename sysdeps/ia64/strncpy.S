/* Optimized version of the standard strncpy() function.
   This file is part of the GNU C Library.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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
        in2:    char count

   If n >= 24, do a memcpy(dest, src, min(strlen(src)+1, n)), followed by a
   memset(dest + strlen(src), 0, n - strlen(src) - 1) if necessary.

   Otherwise, copy characters one by one and fill with nulls if necessary.  */

#include <sysdep.h>
#undef ret

#define saved_b0	loc0
#define saved_pfs	loc1
#define saved_pr	loc2
#define saved_lc	loc3
#define tmp		loc4
#define len		loc5

#define dest		in0
#define src		in1
#define n		in2

#define rc		ret0

ENTRY(strncpy)
	.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(3)
	alloc 	saved_pfs = ar.pfs, 3, 6, 3, 0
	mov	saved_b0 = b0
	.save pr, saved_pr
	mov	saved_pr = pr
	.save ar.lc, saved_lc
	mov	saved_lc = ar.lc
	.body
	cmp.gtu	p6, p0 = 24, n
(p6)	br.cond.spnt .cpyfew
	mov	out0 = src
	mov     tmp = gp ;;
	br.call.sptk.many b0 = strlen# ;; 	// rc = strlen(src);
	add	len = 1, rc			// include the null in len
        mov	gp = tmp
	mov	out0 = dest ;;
	cmp.ltu	p4, p5 = len, n
	mov	out1 = src ;;
(p4)	mov	out2 = len
(p5)	mov	out2 = n
	br.call.sptk.many b0 = memcpy# ;;	// memcpy(dest, src, min(len, n));
	mov     gp = tmp
(p4)	add	out0 = dest, len
(p4)	mov	out1 = r0
(p4)	sub	out2 = n, len
(p4)	br.call.sptk.many b0 = memset# ;;	// fill the rest with nulls
(p4)	mov	gp = tmp
	mov	rc = dest
	mov	b0 = saved_b0
	mov	ar.pfs = saved_pfs
	mov	pr = saved_pr, -1
	br.ret.sptk.many b0
.cpyfew:
	mov	rc = dest
	cmp.eq	p6, p0 = n, r0		
	adds	n = -1, n
(p6)	br.cond.spnt .restore_and_exit ;;	// do nothing if n == 0
	mov	ar.lc = n
	cmp.eq	p6, p0 = r0, r0	;;		// set p6
.loop:
(p6)	ld1	tmp = [src],1 
	;;
	st1	[dest] = tmp, 1
(p6)	cmp.ne	p6, p0 = tmp, r0	// clear p6 after encountering the
	br.cloop.dptk .loop ;;		// null character in src
.restore_and_exit:
	mov	ar.lc = saved_lc
	mov	ar.pfs = saved_pfs
	br.ret.sptk.many b0
END(strncpy)
