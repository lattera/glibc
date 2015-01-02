/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <sysdep.h>

	/* Get address of "sym" in "reg" assuming r51 holds ".Llink". */
	.macro pic_addr reg, sym
#ifdef __tilegx__
	moveli \reg, hw1_last(\sym - .Llink)
	shl16insli \reg, \reg, hw0(\sym - .Llink)
	ADD_PTR \reg, r51, \reg
#else
	ADDLI_PTR \reg, r51, lo16(\sym - .Llink)
	auli \reg, \reg, ha16(\sym - .Llink)
#endif
	.endm

	.text
ENTRY (_start)
	/* Linux starts us with sp pointing at the conventional Elf layout,
	   but we need to allow two 'caller' words for our ABI convention.  */
	{
	 move r52, sp
	 andi sp, sp, -8
	}
	cfi_def_cfa_register (r52)
	{
	 /* Point sp at base of ABI area; point r4 to the caller-sp word. */
	 ADDI_PTR sp, sp, -(2 * REGSIZE)
	 ADDI_PTR r4, sp, -REGSIZE
	}
	{
	 /* Save zero for caller sp in our 'caller' save area, and make
	    sure lr has a zero value, to limit backtraces.  */
	 move lr, zero
	 ST r4, zero
	}
	{
	 move r0, r52
	 jal _dl_start
	}
	/* Save returned start of user program address for later. */
	move r50, r0

	/* See if we were invoked explicitly with the dynamic loader,
	   in which case we have to adjust the argument vector.  */
	lnk r51; .Llink:
	pic_addr r4, _dl_skip_args
	LD4U r4, r4
	BEQZT r4, .Lno_skip

	/* Load the argc word at the initial sp and adjust it.
	   We basically jump "sp" up over the first few argv entries
	   and write "argc" a little higher up in memory, to be the
	   base of the new kernel-initialized stack area.  */
	LD_PTR r0, r52
	{
	 sub r0, r0, r4
	 SHL_PTR_ADD r52, r4, r52
	}
	{
	 ST_PTR r52, r0
	 SHL_PTR_ADD sp, r4, sp
	}
	andi sp, sp, -8

.Lno_skip:
	/* Call_dl_init (_dl_loaded, argc, argv, envp).  See elf/start.s
	   for the layout of memory here; r52 is pointing to "+0".  */
	pic_addr r0, _rtld_local
	{
	 LD_PTR r1, r52  /* load argc in r1 */
	 ADDLI_PTR r2, r52, __SIZEOF_POINTER__  /* point r2 at argv */
	}
	{
	 LD_PTR r0, r0    /* yields _rtld_global._ns_loaded */
	 addi r3, r1, 1
	 move lr, zero
	}
	{
	 SHL_PTR_ADD r3, r3, r2  /* point r3 at envp */
	 jal _dl_init
	}

	/* Call user program whose address we saved in r50.
	   We invoke it just like a static binary, but with _dl_fini
	   in r0 so we can distinguish.  */

	pic_addr r0, _dl_fini
	move lr, zero
	{
	 move sp, r52
	 jr r50
	}

	/* Tell backtracer to give up (_start has no caller). */
	info 2  /* INFO_OP_CANNOT_BACKTRACE */

END (_start)
