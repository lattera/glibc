/* Optimized strlen implementation for PowerPC.
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

	.section	".text"
	.align 2
	.globl strlen
	.type	 strlen,@function
strlen:
	rlwinm 7,3,0,0,29
	lis 8,0x7f7f
	lwz 11,0(7)
	ori 8,8,32639
	rlwinm 4,3,3,27,28
	li 9,-1
	or 10,11,8
	and 0,11,8
	srw 9,9,4
	add 0,0,8
	nor 0,10,0
	and. 11,0,9
	bc 4,2,.L2
.L3:
	lwzu 11,4(7)
	or 10,11,8
	and 0,11,8
	add 0,0,8
	nor. 11,10,0
	bc 12,2,.L3
.L2:
	subf 0,3,7
	cntlzw 3,11
	srwi 3,3,3
	add 3,0,3
	blr
.Lfe1:
	.size	 strlen,.Lfe1-strlen
