/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ralf Baechle <ralf@gnu.org>.

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

#ifndef _SYS_ASM_H
#define _SYS_ASM_H

#include <sgidefs.h>

#ifndef CAT
#ifdef __STDC__
#define __CAT(str1,str2) str1##str2
#else
#define __CAT(str1,str2) str1/**/str2
#endif
#define CAT(str1,str2) __CAT(str1,str2)
#endif

/*
 * Macros to handle different pointer/register sizes for 32/64-bit code
 *
 * 64 bit address space isn't used yet, so we may use the R3000 32 bit
 * defines for now.
 */
#define PTR	.word
#define PTRSIZE	4
#define PTRLOG	2

/*
 * PIC specific declarations
 */
#ifdef __PIC__
#define CPRESTORE(register)                             \
		.cprestore register
#define CPADD(register)                                 \
		.cpadd	register
#define CPLOAD(register)                                \
		.cpload	register
#else
#define CPRESTORE(register)
#define CPADD(register)
#define CPLOAD(register)
#endif

/*
 * LEAF - declare leaf routine
 */
#define	LEAF(symbol)                                    \
		.globl	symbol;                         \
		.align	2;                              \
		.type	symbol,@function;               \
		.ent	symbol,0;                       \
symbol:		.frame	sp,0,ra

/*
 * NESTED - declare nested routine entry point
 */
#define	NESTED(symbol, framesize, rpc)                  \
		.globl	symbol;                         \
		.align	2;                              \
		.type	symbol,@function;               \
		.ent	symbol,0;                       \
symbol:		.frame	sp, framesize, rpc

/*
 * END - mark end of function
 */
#define	END(function)                                   \
		.end	function;		        \
		.size	function,.-function

/*
 * EXPORT - export definition of symbol
 */
#define	EXPORT(symbol)                                  \
		.globl	symbol;                         \
symbol:

/*
 * ABS - export absolute symbol
 */
#define	ABS(symbol,value)                               \
		.globl	symbol;                         \
symbol		=	value

#define	PANIC(msg)                                      \
		.set	push;				\
		.set	reorder;                        \
		la	a0,8f;                          \
		jal	panic;                          \
9:		b	9b;                             \
		.set	pop;				\
		TEXT(msg)

/*
 * Print formated string
 */
#define PRINT(string)                                   \
		.set	push;				\
		.set	reorder;                        \
		la	a0,8f;                          \
		jal	printk;                         \
		.set	pop;				\
		TEXT(string)

#define	TEXT(msg)                                       \
		.data;                                  \
8:		.asciiz	msg;                            \
		.previous;

/*
 * Build text tables
 */
#define TTABLE(string)                                  \
		.text;                                  \
		.word	1f;                             \
		.previous;                              \
		.data;                                  \
1:		.asciz	string;                         \
		.previous

/*
 * MIPS IV pref instruction.
 * Use with .set noreorder only!
 *
 * MIPS IV implementations are free to treat this as a nop.  The R5000
 * is one of them.  So we should have an option not to use this instruction.
 */
#if (_MIPS_ISA == _MIPS_ISA_MIPS4) || (_MIPS_ISA == _MIPS_ISA_MIPS5)
#define PREF(hint,addr)                                 \
		pref	hint,addr
#define PREFX(hint,addr)                                \
		prefx	hint,addr
#else
#define PREF
#define PREFX
#endif

/*
 * MIPS ISA IV/V movn/movz instructions and equivalents for older CPUs.
 */
#if _MIPS_ISA == _MIPS_ISA_MIPS1
#define MOVN(rd,rs,rt)                                  \
		.set	push;				\
		.set	reorder;			\
		beqz	rt,9f;                          \
		move	rd,rs;                          \
		.set	pop;				\
9:
#define MOVZ(rd,rs,rt)                                  \
		.set	push;				\
		.set	reorder;			\
		bnez	rt,9f;                          \
		move	rd,rt;                          \
		.set	pop;				\
9:
#endif /* _MIPS_ISA == _MIPS_ISA_MIPS1 */
#if (_MIPS_ISA == _MIPS_ISA_MIPS2) || (_MIPS_ISA == _MIPS_ISA_MIPS3)
#define MOVN(rd,rs,rt)                                  \
		.set	push;				\
		.set	noreorder;			\
		bnezl	rt,9f;                          \
		move	rd,rs;                          \
		.set	pop;				\
9:
#define MOVZ(rd,rs,rt)                                  \
		.set	push;				\
		.set	noreorder;			\
		beqzl	rt,9f;                          \
		movz	rd,rs;                          \
		.set	pop;				\
9:
#endif /* (_MIPS_ISA == _MIPS_ISA_MIPS2) || (_MIPS_ISA == _MIPS_ISA_MIPS3) */
#if (_MIPS_ISA == _MIPS_ISA_MIPS4) || (_MIPS_ISA == _MIPS_ISA_MIPS5)
#define MOVN(rd,rs,rt)                                  \
		movn	rd,rs,rt
#define MOVZ(rd,rs,rt)                                  \
		movz	rd,rs,rt
#endif /* (_MIPS_ISA == _MIPS_ISA_MIPS4) || (_MIPS_ISA == _MIPS_ISA_MIPS5) */

/*
 * Stack alignment
 */
#if (_MIPS_ISA == _MIPS_ISA_MIPS1) || (_MIPS_ISA == _MIPS_ISA_MIPS2)
#define ALSZ	7
#define ALMASK	~7
#endif
#if (_MIPS_ISA == _MIPS_ISA_MIPS3) || (_MIPS_ISA == _MIPS_ISA_MIPS4) || \
    (_MIPS_ISA == _MIPS_ISA_MIPS5)
#define ALSZ	15
#define ALMASK	~15
#endif

/*
 * Size of a register
 */
#ifdef __mips64
#define SZREG	8
#else
#define SZREG	4
#endif

/*
 * Use the following macros in assemblercode to load/store registers,
 * pointers etc.
 */
#if (_MIPS_ISA == _MIPS_ISA_MIPS1) || (_MIPS_ISA == _MIPS_ISA_MIPS2)
#define REG_S sw
#define REG_L lw
#define PTR_SUBU subu
#define PTR_ADDU addu
#endif
#if (_MIPS_ISA == _MIPS_ISA_MIPS3) || (_MIPS_ISA == _MIPS_ISA_MIPS4) || \
    (_MIPS_ISA == _MIPS_ISA_MIPS5)
#define REG_S sd
#define REG_L ld
/* We still live in a 32 bit address space ...  */
#define PTR_SUBU subu
#define PTR_ADDU addu
#endif

/*
 * How to add/sub/load/store/shift C int variables.
 */
#if (_MIPS_SZINT == 32)
#define INT_ADD	add
#define INT_ADDI	addi
#define INT_ADDU	addu
#define INT_ADDIU	addiu
#define INT_SUB	add
#define INT_SUBI	subi
#define INT_SUBU	subu
#define INT_SUBIU	subu
#define INT_L		lw
#define INT_S		sw
#define LONG_SLL	sll
#define LONG_SLLV	sllv
#define LONG_SRL	srl
#define LONG_SRLV	srlv
#define LONG_SRA	sra
#define LONG_SRAV	srav
#endif

#if (_MIPS_SZINT == 64)
#define INT_ADD	dadd
#define INT_ADDI	daddi
#define INT_ADDU	daddu
#define INT_ADDIU	daddiu
#define INT_SUB	dadd
#define INT_SUBI	dsubi
#define INT_SUBU	dsubu
#define INT_SUBIU	dsubu
#define INT_L		ld
#define INT_S		sd
#define LONG_SLL	dsll
#define LONG_SLLV	dsllv
#define LONG_SRL	dsrl
#define LONG_SRLV	dsrlv
#define LONG_SRA	dsra
#define LONG_SRAV	dsrav
#endif

/*
 * How to add/sub/load/store/shift C long variables.
 */
#if (_MIPS_SZLONG == 32)
#define LONG_ADD	add
#define LONG_ADDI	addi
#define LONG_ADDU	addu
#define LONG_ADDIU	addiu
#define LONG_SUB	add
#define LONG_SUBI	subi
#define LONG_SUBU	subu
#define LONG_SUBIU	subu
#define LONG_L		lw
#define LONG_S		sw
#define LONG_SLL	sll
#define LONG_SLLV	sllv
#define LONG_SRL	srl
#define LONG_SRLV	srlv
#define LONG_SRA	sra
#define LONG_SRAV	srav
#endif

#if (_MIPS_SZLONG == 64)
#define LONG_ADD	dadd
#define LONG_ADDI	daddi
#define LONG_ADDU	daddu
#define LONG_ADDIU	daddiu
#define LONG_SUB	dadd
#define LONG_SUBI	dsubi
#define LONG_SUBU	dsubu
#define LONG_SUBIU	dsubu
#define LONG_L		ld
#define LONG_S		sd
#define LONG_SLL	dsll
#define LONG_SLLV	dsllv
#define LONG_SRL	dsrl
#define LONG_SRLV	dsrlv
#define LONG_SRA	dsra
#define LONG_SRAV	dsrav
#endif

/*
 * How to add/sub/load/store/shift pointers.
 */
#if (_MIPS_SZLONG == 32)
#define PTR_ADD	add
#define PTR_ADDI	addi
#define PTR_ADDU	addu
#define PTR_ADDIU	addiu
#define PTR_SUB		add
#define PTR_SUBI	subi
#define PTR_SUBU	subu
#define PTR_SUBIU	subu
#define PTR_L		lw
#define PTR_S		sw
#define PTR_SLL		sll
#define PTR_SLLV	sllv
#define PTR_SRL		srl
#define PTR_SRLV	srlv
#define PTR_SRA		sra
#define PTR_SRAV	srav

#define PTR_SCALESHIFT	2
#endif

#if (_MIPS_SZLONG == 64)
#define PTR_ADD	dadd
#define PTR_ADDI	daddi
#define PTR_ADDU	daddu
#define PTR_ADDIU	daddiu
#define PTR_SUB		dadd
#define PTR_SUBI	dsubi
#define PTR_SUBU	dsubu
#define PTR_SUBIU	dsubu
#define PTR_L		ld
#define PTR_S		sd
#define PTR_SLL		dsll
#define PTR_SLLV	dsllv
#define PTR_SRL		dsrl
#define PTR_SRLV	dsrlv
#define PTR_SRA		dsra
#define PTR_SRAV	dsrav

#define PTR_SCALESHIFT	3
#endif

/*
 * Some cp0 registers were extended to 64bit for MIPS III.
 */
#if (_MIPS_ISA == _MIPS_ISA_MIPS1) || (_MIPS_ISA == _MIPS_ISA_MIPS2)
#define MFC0	mfc0
#define MTC0	mtc0
#endif
#if (_MIPS_ISA == _MIPS_ISA_MIPS3) || (_MIPS_ISA == _MIPS_ISA_MIPS4) || \
    (_MIPS_ISA == _MIPS_ISA_MIPS5)
#define MFC0	dmfc0
#define MTC0	dmtc0
#endif

#endif /* sys/asm.h */
