/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <sysdeps/unix/powerpc/sysdep.h>


/* This seems to always be the case on PPC.  */
#define ALIGNARG(log2) log2

/* How to define local lables.  */
#define L(name) L..##name

/* Label in text section.  */
#define C_TEXT(name) .##name

/* Function descriptor.  */
#define FUNCDESC(real, code) \
  .toc;									      \
  .csect real##[DS], 3;							      \
  .globl real;								      \
real:									      \
  .long code, TOC[tc0], 0;

/* Code to generate function entry code.  */
#define ENTRY(name) \
  FUNCDESC (name, C_TEXT (name))					      \
  .csect .text[PR], 2;							      \
  .globl C_TEXT (name);							      \
C_TEXT (name):

/* XXX For now we don't define any code.  */
#define CALL_MCOUNT

#define EALIGN_W_0  /* No words to insert.  */
#define EALIGN_W_1  nop
#define EALIGN_W_2  nop;nop
#define EALIGN_W_3  nop;nop;nop
#define EALIGN_W_4  EALIGN_W_3;nop
#define EALIGN_W_5  EALIGN_W_4;nop
#define EALIGN_W_6  EALIGN_W_5;nop
#define EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#ifdef PROF
#define EALIGN(name, alignt, words)					      \
  FUNCDESC (name, C_TEXT (name))					      \
  .csect .text[PR], 2;							      \
  .align ALIGNARG(2);							      \
  .globl C_TEXT (name);							      \
C_TEXT (name):								      \
  CALL_MCOUNT								      \
  b L(align_0);								      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
L(align_0):
#else /* PROF */
#define EALIGN(name, alignt, words)					      \
  FUNCDESC (name, C_TEXT (name))					      \
  .csect .text[PR], 2;							      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  .globl C_TEXT (name);							      \
C_TEXT (name):
#endif

/* No special end code for now.  We will eventually add to usual prolog
   with function length etc.  */
#define END(name)


/* Jumping to another function.  We are jumping to the TOC entry.  */
#define JUMPTARGET(name) C_TEXT (name)
