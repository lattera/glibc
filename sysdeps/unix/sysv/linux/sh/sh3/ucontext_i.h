/* Offsets and other constants needed in the *context() function
   implementation.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

#define SIG_BLOCK	0
#define SIG_SETMASK	2

/* Offsets of the fields in the ucontext_t structure.  */
#define oLINK		4
#define oSS_SP		8
#define oSS_SIZE	16
#define oR0		24
#define oR1		28
#define oR2		32
#define oR3		36
#define oR4		40
#define oR5		44
#define oR6		48
#define oR7		52
#define oR8		56
#define oR9		60
#define oR10		64
#define oR11		68
#define oR12		72
#define oR13		76
#define oR14		80
#define oR15		84
#define oPC		88
#define oPR		92
#define oSR		96
#define oGBR		100
#define oMACH		104
#define oMACL		108
#define oSIGMASK	112

/* Tests run in stdlib/tst-ucontext-off.  */
#define TESTS \
  TEST (uc_link, oLINK);				\
  TEST (uc_stack.ss_sp, oSS_SP);			\
  TEST (uc_stack.ss_size, oSS_SIZE);			\
  TEST (uc_mcontext.gregs[R0], oR0);			\
  TEST (uc_mcontext.gregs[R1], oR1);			\
  TEST (uc_mcontext.gregs[R2], oR2);			\
  TEST (uc_mcontext.gregs[R3], oR3);			\
  TEST (uc_mcontext.gregs[R4], oR4);			\
  TEST (uc_mcontext.gregs[R5], oR5);			\
  TEST (uc_mcontext.gregs[R6], oR6);			\
  TEST (uc_mcontext.gregs[R7], oR7);			\
  TEST (uc_mcontext.gregs[R8], oR8);			\
  TEST (uc_mcontext.gregs[R9], oR9);			\
  TEST (uc_mcontext.gregs[R10], oR10);			\
  TEST (uc_mcontext.gregs[R11], oR11);			\
  TEST (uc_mcontext.gregs[R12], oR12);			\
  TEST (uc_mcontext.gregs[R13], oR13);			\
  TEST (uc_mcontext.gregs[R14], oR14);			\
  TEST (uc_mcontext.gregs[R15], oR15);			\
  TEST (uc_mcontext.pc, oPC);				\
  TEST (uc_mcontext.pr, oPR);				\
  TEST (uc_mcontext.sr, oSR);				\
  TEST (uc_mcontext.gbr, oGBR);				\
  TEST (uc_mcontext.mach, oMACH);			\
  TEST (uc_mcontext.macl, oMACL);			\
  TEST (uc_sigmask, oSIGMASK);
