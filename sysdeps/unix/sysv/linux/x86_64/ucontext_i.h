/* Offsets and other constants needed in the *context() function
   implementation for Linux/x86-64.
   Copyright (C) 2002, 2005 Free Software Foundation, Inc.
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

/* Since we cannot include a header to define _NSIG/8, we define it
   here.  */
#define _NSIG8		8

/* Offsets of the fields in the ucontext_t structure.  */
#define oRBP		120
#define oRSP		160
#define oRBX		128
#define oR8		40
#define oR9		48
#define oR12		72
#define oR13		80
#define oR14		88
#define oR15		96
#define oRDI		104
#define oRSI		112
#define oRDX		136
#define oRAX		144
#define oRCX		152
#define oRIP		168
#define oFPREGS		224
#define oSIGMASK	296
#define oFPREGSMEM	424
#define oMXCSR		448

/* Tests run in stdlib/tst-ucontext-off.  */
#define TESTS \
  TEST (uc_mcontext.gregs[REG_RBP], oRBP);				\
  TEST (uc_mcontext.gregs[REG_RSP], oRSP);				\
  TEST (uc_mcontext.gregs[REG_RBX], oRBX);				\
  TEST (uc_mcontext.gregs[REG_R8], oR8);				\
  TEST (uc_mcontext.gregs[REG_R9], oR9);				\
  TEST (uc_mcontext.gregs[REG_R12], oR12);				\
  TEST (uc_mcontext.gregs[REG_R13], oR13);				\
  TEST (uc_mcontext.gregs[REG_R14], oR14);				\
  TEST (uc_mcontext.gregs[REG_R15], oR15);				\
  TEST (uc_mcontext.gregs[REG_RDI], oRDI);				\
  TEST (uc_mcontext.gregs[REG_RSI], oRSI);				\
  TEST (uc_mcontext.gregs[REG_RDX], oRDX);				\
  TEST (uc_mcontext.gregs[REG_RAX], oRAX);				\
  TEST (uc_mcontext.gregs[REG_RCX], oRCX);				\
  TEST (uc_mcontext.gregs[REG_RIP], oRIP);				\
  TEST (uc_mcontext.fpregs, oFPREGS);					\
  TEST (uc_sigmask, oSIGMASK);						\
  TEST (__fpregs_mem, oFPREGSMEM);					\
  TEST (__fpregs_mem.mxcsr, oMXCSR);
