/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.

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

#include <sysdep.h>

#include <asm/unistd.h>
#include <asm/fpu.h>

/*
 * Arguments for __libc_start_main:
 *	out0:	main
 *	out1:	argc
 *	out2:	argv
 *	out3:	init
 *	out4:	fini
 *	out5:	rtld_fini
 *	out6:	stack_end
 */

	.align 32
	.global _start

	.proc _start
	type _start,@function
_start:
	.prologue
	{ .mlx
	  alloc r2 = ar.pfs,0,0,7,0
	  movl r3 = FPSR_DEFAULT
	}
	{ .mlx
	  adds out2 = 16, sp	/* get address of argc value */
	  movl gp = @gprel(0f)
	  ;;
	}
0:	{ .mmi
	  ld8 out1 = [out2], 8	/* load argc and move out2 to become argv */
	  mov.m r10 = ar.bsp	/* fetch rbs base address */
	  mov r9 = ip
	  ;;
	}
	{ .mii
	  mov ar.fpsr = r3
	  sub gp = r9, gp	/* back-compute gp value */
	  adds out6 = 16, sp	/* highest non-environment stack address */
	  ;;
	}
	{
	  addl r11 = @ltoff(__libc_ia64_register_backing_store_base), gp
	  addl out0 = @ltoff(@fptr(main)), gp
	  addl out3 = @ltoff(@fptr(_init)), gp
	  ;;
	}
	{ .mmi
	  ld8 r3 = [r11]	/* pointer to __libc_ia64_register_backing_store_base */
	  ld8 out0 = [out0]	/* pointer to `main' function descriptor */
	  addl out4 = @ltoff(@fptr(_fini)), gp
	  ;;
	}
	{ .mmi
	  ld8 out3 = [out3]	/* pointer to `init' function descriptor */
	  ld8 out4 = [out4]	/* pointer to `fini' function descriptor */
	  .save rp, r4
	  mov r4 = r0		/* terminate unwind chain with a NULL return-pointer */
	}
	.body
	{ .mib
	  st8 [r3] = r10
	  mov out5 = ret0	/* dynamic linker destructor */
	  br.call.sptk.few rp = __libc_start_main
	}
	{ .mib
	  break 0	/* break miserably if we ever return */
	}
	.endp _start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

	.common __libc_ia64_register_backing_store_base, 8, 8
