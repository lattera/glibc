/* Definitions for x86 syntax variations.
   Copyright (C) 1992, 1994, 1995, 1997 Free Software Foundation, Inc.

   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.  The master source lives in the GNU MP Library.

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

#undef ALIGN

#ifdef GAS_SYNTAX
#define R(r) %r
#define MEM(base)(base)
#define MEM_DISP(base,displacement)displacement(R(base))
#define MEM_INDEX(base,index,size)(R(base),R(index),size)
#ifdef __STDC__
#define INSN1(mnemonic,size_suffix,dst)mnemonic##size_suffix dst
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic##size_suffix src,dst
#else
#define INSN1(mnemonic,size_suffix,dst)mnemonic/**/size_suffix dst
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic/**/size_suffix src,dst
#endif
#define TEXT .text
#define ALIGN(log) .align log
#define GLOBL .globl
#endif

#ifdef INTEL_SYNTAX
#define R(r) r
#define MEM(base)[base]
#define MEM_DISP(base,displacement)[base+(displacement)]
#define MEM_INDEX(base,index,size)[base+index*size]
#define INSN1(mnemonic,size_suffix,dst)mnemonic dst
#define INSN2(mnemonic,size_suffix,dst,src)mnemonic dst,src
#define TEXT .text
#define ALIGN(log) .align log
#define GLOBL .globl
#endif

#ifdef BROKEN_ALIGN
#undef ALIGN
#define ALIGN(log) .align log,0x90
#endif

#if !defined (NOLOG_ALIGN) && defined (HAVE_ELF)
#define NOLOG_ALIGN
#endif

#ifdef NOLOG_ALIGN
#undef ALIGN
#define ALIGN(log) .align 1<<log
#endif

#undef L
#ifdef __ELF__
#ifdef __STDC__
#define L(body)	.L##body
#else
#define L(body)	.L/**/body
#endif
#else
#ifdef __STDC__
#define L(body) L##body
#else
#define L(body) L/**/body
#endif
#endif
