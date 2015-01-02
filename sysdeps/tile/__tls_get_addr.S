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
#include <tls.h>
#include <bits/wordsize.h>

#if __WORDSIZE == 64
#define LOG_SIZEOF_DTV_T 4
#else
#define LOG_SIZEOF_DTV_T 3
#endif

/* On entry, r0 points to two words, the module and the offset.
   On return, r0 holds the pointer to the relevant TLS memory.
   Only registers r25..r29 are clobbered by the call.  */

	.text
ENTRY (__tls_get_addr)
	{
	 lnk r25
	 ADDI_PTR r27, tp, DTV_OFFSET
	}
.Llnk:
#ifdef __tilegx__
	{
	 LD_PTR r27, r27	/* r27 = THREAD_DTV() */
	 moveli r26, hw1_last(_rtld_local + TLS_GENERATION_OFFSET - .Llnk)
	}
	shl16insli r26, r26, hw0(_rtld_local + TLS_GENERATION_OFFSET - .Llnk)
	{
	 ADD_PTR r25, r25, r26
	 LD_PTR r26, r0		/* r26 = ti_module */
	}
#else
	{
	 LD_PTR r27, r27	/* r27 = THREAD_DTV() */
	 addli r25, r25, lo16(_rtld_local + TLS_GENERATION_OFFSET - .Llnk)
	}
	{
	 auli r25, r25, ha16(_rtld_local + TLS_GENERATION_OFFSET - .Llnk)
	 LD_PTR r26, r0		/* r26 = ti_module */
	}
#endif
	LD_PTR r25, r25		/* r25 = DL(dl_tls_generation) */
	{
	 LD_PTR r28, r27	/* r28 = THREAD_DTV()->counter */
	 ADDI_PTR r29, r0, __SIZEOF_POINTER__
	}
	{
	 LD_PTR r29, r29	/* r29 = ti_offset */
	 CMPEQ r25, r28, r25	/* r25 nonzero if generation OK */
	 shli r28, r26, LOG_SIZEOF_DTV_T  /* byte index into dtv array */
	}
	{
	 BEQZ r25, .Lslowpath
	 CMPEQI r25, r26, -1	/* r25 nonzero if ti_module invalid */
	}
	{
	 BNEZ r25, .Lslowpath
	 ADD_PTR r28, r28, r27	/* pointer into module array */
	}
	LD_PTR r26, r28		/* r26 = module TLS pointer */
	CMPEQI r25, r26, -1     /* check r26 == TLS_DTV_UNALLOCATED */
	BNEZ r25, .Lslowpath
	{
	 ADD_PTR r0, r26, r29
	 jrp lr
	}

.Lslowpath:
	{
	 ST sp, lr
	 ADDLI_PTR r29, sp, - (25 * REGSIZE)
	}
	cfi_offset (lr, 0)
	{
	 ST r29, sp
	 ADDLI_PTR sp, sp, - (26 * REGSIZE)
	}
	cfi_def_cfa_offset (26 * REGSIZE)
	ADDI_PTR r29, sp, (2 * REGSIZE)
	{ ST r29, r1;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r2;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r3;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r4;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r5;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r6;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r7;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r8;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r9;  ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r10; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r11; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r12; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r13; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r14; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r15; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r16; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r17; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r18; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r19; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r20; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r21; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r22; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r23; ADDI_PTR r29, r29, REGSIZE }
	{ ST r29, r24; ADDI_PTR r29, r29, REGSIZE }
	.hidden __tls_get_addr_slow
	jal __tls_get_addr_slow
	ADDI_PTR r29, sp, (2 * REGSIZE)
	{ LD r1,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r2,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r3,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r4,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r5,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r6,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r7,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r8,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r9,  r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r10, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r11, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r12, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r13, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r14, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r15, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r16, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r17, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r18, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r19, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r20, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r21, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r22, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r23, r29; ADDI_PTR r29, r29, REGSIZE }
	{ LD r24, r29; ADDLI_PTR sp, sp, (26 * REGSIZE) }
	cfi_def_cfa_offset (0)
	LD lr, sp
	jrp lr
END (__tls_get_addr)
