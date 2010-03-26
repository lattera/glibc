/* Implementation of profiling support.  ARM EABI version.
   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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

/* Don't call mcount when calling mcount...  */
#undef PROF

#include <sysdep.h>


#ifdef __thumb2__
	.thumb
	.syntax unified
#endif


/* Use an assembly stub with a special ABI.  The calling lr has been
   pushed to the stack (which will be misaligned).  We should preserve
   all registers except ip and pop a word off the stack.

   NOTE: This assumes mcount_internal does not clobber any non-core
   (coprocessor) registers.  Currently this is true, but may require
   additional attention in the future.

   The calling sequence looks something like:
func:
   push {lr}
   bl __gnu_mount_nc
   <function body>
*/

ENTRY(__gnu_mcount_nc)
	push {r0, r1, r2, r3, lr}
	cfi_adjust_cfa_offset (20)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r2, 8)
	cfi_rel_offset (r3, 12)
	cfi_rel_offset (lr, 16)
	bic r1, lr, #1
	ldr r0, [sp, #20]
	bl __mcount_internal
	pop {r0, r1, r2, r3, ip, lr}
	cfi_adjust_cfa_offset (-24)
	cfi_restore (r0)
	cfi_restore (r1)
	cfi_restore (r2)
	cfi_restore (r3)
	cfi_register (lr, ip)
	bx ip
END(__gnu_mcount_nc)


/* Provide old mcount for backwards compatibility.  This requires
   code be compiled with APCS frame pointers.  */

ENTRY(_mcount)
	stmdb sp!, {r0, r1, r2, r3, fp, lr}
	cfi_adjust_cfa_offset (24)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r2, 8)
	cfi_rel_offset (r3, 12)
	cfi_rel_offset (fp, 16)
	cfi_rel_offset (lr, 20)
#ifdef __thumb2__
	movs r0, fp
	ittt ne
	ldrne r0, [r0, #-4]
#else
	movs fp, fp
	ldrne r0, [fp, #-4]
#endif
	movnes r1, lr
	blne __mcount_internal
#ifdef __thumb2__
	ldmia sp!, {r0, r1, r2, r3, fp, pc}
#else
	ldmia sp!, {r0, r1, r2, r3, fp, lr}
	cfi_adjust_cfa_offset (-24)
	cfi_restore (r0)
	cfi_restore (r1)
	cfi_restore (r2)
	cfi_restore (r3)
	cfi_restore (fp)
	cfi_restore (lr)
	bx lr
#endif
END(_mcount)

/* The canonical name for the function is `_mcount' in both C and asm,
   but some old asm code might assume it's `mcount'.  */
#undef mcount
weak_alias (_mcount, mcount)
