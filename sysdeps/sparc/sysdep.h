/* Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Bits present in AT_HWCAP on SPARC.  */

#define HWCAP_SPARC_FLUSH	0x00000001
#define HWCAP_SPARC_STBAR	0x00000002
#define HWCAP_SPARC_SWAP	0x00000004
#define HWCAP_SPARC_MULDIV	0x00000008
#define HWCAP_SPARC_V9		0x00000010
#define HWCAP_SPARC_ULTRA3	0x00000020
#define HWCAP_SPARC_BLKINIT	0x00000040
#define HWCAP_SPARC_N2		0x00000080
#define HWCAP_SPARC_MUL32	0x00000100
#define HWCAP_SPARC_DIV32	0x00000200
#define HWCAP_SPARC_FSMULD	0x00000400
#define HWCAP_SPARC_V8PLUS	0x00000800
#define HWCAP_SPARC_POPC	0x00001000
#define HWCAP_SPARC_VIS		0x00002000
#define HWCAP_SPARC_VIS2	0x00004000
#define HWCAP_SPARC_ASI_BLK_INIT 0x00008000
#define HWCAP_SPARC_FMAF	0x00010000
#define HWCAP_SPARC_VIS3	0x00020000
#define HWCAP_SPARC_HPC		0x00040000
#define HWCAP_SPARC_RANDOM	0x00080000
#define HWCAP_SPARC_TRANS	0x00100000
#define HWCAP_SPARC_FJFMAU	0x00200000
#define HWCAP_SPARC_IMA		0x00400000
#define HWCAP_SPARC_ASI_CACHE_SPARING \
				0x00800000

#ifdef	__ASSEMBLER__

#define SPARC_PIC_THUNK(reg)						\
	.ifndef __sparc_get_pc_thunk.reg;				\
	.section .text.__sparc_get_pc_thunk.reg,"axG",@progbits,__sparc_get_pc_thunk.reg,comdat; \
	.align	 32;							\
	.weak	 __sparc_get_pc_thunk.reg;				\
	.hidden	 __sparc_get_pc_thunk.reg;				\
	.type	 __sparc_get_pc_thunk.reg, #function;			\
__sparc_get_pc_thunk.reg:		   				\
	jmp	%o7 + 8;						\
	 add	%o7, %reg, %##reg;					\
	.previous;							\
	.endif;

/* Even when v9 we use a call sequence instead of using "rd %pc" because
   RDPC is extremely expensive and incurs a full pipeline flush.  */

#define SETUP_PIC_REG(reg)						\
	SPARC_PIC_THUNK(reg)						\
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %##reg;			\
	call	__sparc_get_pc_thunk.reg;				\
	 or	%##reg, %lo(_GLOBAL_OFFSET_TABLE_+4), %##reg;

#define SETUP_PIC_REG_LEAF(reg, tmp)					\
	SPARC_PIC_THUNK(reg)						\
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %##reg;			\
	mov	%o7, %##tmp;		      				\
	call	__sparc_get_pc_thunk.reg;				\
	 or	%##reg, %lo(_GLOBAL_OFFSET_TABLE_+4), %##reg;		\
	mov	%##tmp, %o7;

#undef ENTRY
#define ENTRY(name)			\
	.align	4;			\
	.global	C_SYMBOL_NAME(name);	\
	.type	name, @function;	\
C_LABEL(name)				\
	cfi_startproc;

#undef END
#define END(name)			\
	cfi_endproc;			\
	.size name, . - name

#undef LOC
#define LOC(name)  .L##name

#endif	/* __ASSEMBLER__ */
