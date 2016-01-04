/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.
   Based on work contributed by David Mosberger (davidm@cs.arizona.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Assembly stub to invoke __mcount_internal().  Compiler-generated
   code calls mcount after executing a function's prologue, placing
   the "lr" register in "r10" for the call.  As a result "lr" is the
   function that invoked mcount, and "r10" is mcount's caller's
   caller.  However, we have to save all the parameter registers here
   before invoking _mcount_internal.  Callee-save and temporary
   registers need no special attention.  We save r10 and restore it to
   lr on the way out, to properly handle the case of ENTRY() in
   assembly code, before lr is saved.  We use the name __mcount since
   the gcc community prefers using the reserved namespace.  */

#include <sysdep.h>

	.text
ENTRY(__mcount)
	{
	 ST sp, lr
	 ADDI_PTR r29, sp, - (12 * REGSIZE)
	}
	cfi_offset (lr, 0)
	{
	 ADDI_PTR sp, sp, - (13 * REGSIZE)
	 ST r29, sp
	 ADDI_PTR r29, r29, REGSIZE
	}
	cfi_def_cfa_offset (13 * REGSIZE)
	{ ST r29, r0; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r1; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r2; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r3; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r4; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r5; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r6; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r7; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r8; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r9; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r10; ADDI_PTR r29, r29, REGSIZE; move r0, r10 }
	{
	 move r1, lr
	 jal __mcount_internal
	}
	{
	 ADDI_PTR r29, sp, (2 * REGSIZE)
	}
	{ LD r0, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r1, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r2, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r3, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r4, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r5, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r6, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r7, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r8, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r9, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r10, r29; ADDI_PTR sp, sp, (13 * REGSIZE) }
	cfi_def_cfa_offset (0)
	{
	 LD lr, sp
	}
	{
	 move lr, r10
	 jrp lr
	}
END(__mcount)

#undef mcount
weak_alias (__mcount, _mcount)   /* exported in gmon/Versions */
weak_alias (__mcount, mcount)    /* exported in stdlib/Versions */
