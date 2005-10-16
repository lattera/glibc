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
#define oFR0		112
#define oFR1		116
#define oFR2		120
#define oFR3		124
#define oFR4		128
#define oFR5		132
#define oFR6		136
#define oFR7		140
#define oFR8		144
#define oFR9		148
#define oFR10		152
#define oFR11		156
#define oFR12		160
#define oFR13		164
#define oFR14		168
#define oFR15		172
#define oXFR0		176
#define oXFR1		180
#define oXFR2		184
#define oXFR3		188
#define oXFR4		192
#define oXFR5		196
#define oXFR6		200
#define oXFR7		204
#define oXFR8		208
#define oXFR9		212
#define oXFR10		216
#define oXFR11		220
#define oXFR12		224
#define oXFR13		228
#define oXFR14		232
#define oXFR15		236
#define oFPSCR		240
#define oFPUL		244
#define oOWNEDFP	248
#define oSIGMASK	252

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
  TEST (uc_mcontext.fpregs[0], oFR0);			\
  TEST (uc_mcontext.fpregs[1], oFR1);			\
  TEST (uc_mcontext.fpregs[2], oFR2);			\
  TEST (uc_mcontext.fpregs[3], oFR3);			\
  TEST (uc_mcontext.fpregs[4], oFR4);			\
  TEST (uc_mcontext.fpregs[5], oFR5);			\
  TEST (uc_mcontext.fpregs[6], oFR6);			\
  TEST (uc_mcontext.fpregs[7], oFR7);			\
  TEST (uc_mcontext.fpregs[8], oFR8);			\
  TEST (uc_mcontext.fpregs[9], oFR9);			\
  TEST (uc_mcontext.fpregs[10], oFR10);			\
  TEST (uc_mcontext.fpregs[11], oFR11);			\
  TEST (uc_mcontext.fpregs[12], oFR12);			\
  TEST (uc_mcontext.fpregs[13], oFR13);			\
  TEST (uc_mcontext.fpregs[14], oFR14);			\
  TEST (uc_mcontext.fpregs[15], oFR15);			\
  TEST (uc_mcontext.xfpregs[0], oXFR0);			\
  TEST (uc_mcontext.xfpregs[1], oXFR1);			\
  TEST (uc_mcontext.xfpregs[2], oXFR2);			\
  TEST (uc_mcontext.xfpregs[3], oXFR3);			\
  TEST (uc_mcontext.xfpregs[4], oXFR4);			\
  TEST (uc_mcontext.xfpregs[5], oXFR5);			\
  TEST (uc_mcontext.xfpregs[6], oXFR6);			\
  TEST (uc_mcontext.xfpregs[7], oXFR7);			\
  TEST (uc_mcontext.xfpregs[8], oXFR8);			\
  TEST (uc_mcontext.xfpregs[9], oXFR9);			\
  TEST (uc_mcontext.xfpregs[10], oXFR10);		\
  TEST (uc_mcontext.xfpregs[11], oXFR11);		\
  TEST (uc_mcontext.xfpregs[12], oXFR12);		\
  TEST (uc_mcontext.xfpregs[13], oXFR13);		\
  TEST (uc_mcontext.xfpregs[14], oXFR14);		\
  TEST (uc_mcontext.xfpregs[15], oXFR15);		\
  TEST (uc_mcontext.fpscr, oFPSCR);			\
  TEST (uc_mcontext.fpul, oFPUL);			\
  TEST (uc_mcontext.ownedfp, oOWNEDFP);			\
  TEST (uc_sigmask, oSIGMASK);
