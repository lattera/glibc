/* asm.h -- Definitions for 68k syntax variations.

Copyright (C) 1992, 1994 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifdef MIT_SYNTAX
#define MEM(base)base@
#define MEM_DISP(base,displacement)base@(displacement)
#define MEM_PREDEC(memory_base)memory_base@-
#define MEM_POSTINC(memory_base)memory_base@+
#ifdef __STDC__
#define INSN1(mnemonic,size_suffix,dst)mnemonic##size_suffix dst
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic##size_suffix src,dst
#else
#define INSN1(mnemonic,size_suffix,dst)mnemonic/**/size_suffix dst
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic/**/size_suffix src,dst
#endif
#define LAB(label) label:
#define TEXT .text
#define ALIGN .even
#define GLOBL .globl
#endif

#ifdef SONY_SYNTAX
#define MEM(base)(base)
#define MEM_DISP(base,displacement)(displacement,base)
#define MEM_PREDEC(memory_base)-(memory_base)
#define MEM_POSTINC(memory_base)(memory_base)+
#define INSN1(mnemonic,size_suffix,dst)mnemonic.size_suffix dst
#ifdef __STDC__
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic.size_suffix src##,dst
#else
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic.size_suffix src/**/,dst
#endif
#define LAB(label) label:
#define TEXT .text
#define ALIGN .even
#define GLOBL .globl
#endif

#ifdef MOTOROLA_SYNTAX
#define MEM(base)(base)
#define MEM_DISP(base,displacement)(displacement,base)
#define MEM_PREDEC(memory_base)-(memory_base)
#define MEM_POSTINC(memory_base)(memory_base)+
#define INSN1(mnemonic,size_suffix,dst)mnemonic.size_suffix dst
#ifdef __STDC__
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic.size_suffix src##,dst
#else
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic.size_suffix src/**/,dst
#endif
#define LAB(label) label
#define TEXT
#define ALIGN
#define GLOBL XDEF
#define l L
#define w W
#define move MOVE
#define eor EOR
#define lsr LSR
#define add ADD
#define addx ADDX
#define addq ADDQ
#define sub SUB
#define subx SUBX
#define subq SUBQ
#define neg NEG
#define bcc BCC
#define bcs BCS
#define bra BRA
#define dbf DBF
#define rts RTS
#define d0 D0
#define d1 D1
#define d2 D2
#define d3 D3
#define d4 D4
#define d5 D5
#define d6 D6
#define d7 D7
#define a0 A0
#define a1 A1
#define a2 A2
#define a3 A3
#define a4 A4
#define a5 A5
#define a6 A6
#define a7 A7
#define sp SP
#endif
