/* Machine-dependent definitions for profiling support.  ARM EABI version.
   Copyright (C) 2008 Free Software Foundation, Inc.
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

/* GCC for the ARM cannot compile __builtin_return_address(N) for N != 0, 
   so we must use an assembly stub.  */

#include <sysdep.h>
static void mcount_internal (u_long frompc, u_long selfpc) __attribute_used__;

#define _MCOUNT_DECL(frompc, selfpc) \
static void mcount_internal (u_long frompc, u_long selfpc)

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


#define MCOUNT								\
void __attribute__((__naked__)) __gnu_mcount_nc(void)			\
{									\
    asm ("push {r0, r1, r2, r3, lr}\n\t"				\
	 "bic r1, lr, #1\n\t"						\
	 "ldr r0, [sp, #20]\n\t"					\
	 "bl mcount_internal\n\t"					\
	 "pop {r0, r1, r2, r3, ip, lr}\n\t"				\
	 "bx ip");							\
}									\
OLD_MCOUNT

/* Provide old mcount for backwards compatibility.  This requires
   code be compiled with APCS frame pointers.  */

#ifndef NO_UNDERSCORES
/* The asm symbols for C functions are `_function'.
   The canonical name for the counter function is `mcount', no _.  */
void _mcount (void) asm ("mcount");
#else
/* The canonical name for the function is `_mcount' in both C and asm,
   but some old asm code might assume it's `mcount'.  */
void _mcount (void);
weak_alias (_mcount, mcount)
#endif

#ifdef __thumb2__

#define OLD_MCOUNT							\
void __attribute__((__naked__)) _mcount (void)				\
{									\
  __asm__("push		{r0, r1, r2, r3, fp, lr};"			\
	  "movs		r0, fp;"					\
	  "ittt		ne;"						\
	  "ldrne	r0, [r0, #-4];"					\
	  "movsne	r1, lr;"					\
	  "blne		mcount_internal;"				\
	  "pop		{r0, r1, r2, r3, fp, pc}");			\
}

#else

#define OLD_MCOUNT							\
void __attribute__((__naked__)) _mcount (void)				\
{									\
  __asm__("stmdb	sp!, {r0, r1, r2, r3, fp, lr};"			\
	  "movs		fp, fp;"					\
	  "ldrne	r0, [fp, #-4];"					\
	  "movnes	r1, lr;"					\
	  "blne		mcount_internal;"				\
	  "ldmia	sp!, {r0, r1, r2, r3, fp, lr};"			\
	  "bx		lr");						\
}

#endif
