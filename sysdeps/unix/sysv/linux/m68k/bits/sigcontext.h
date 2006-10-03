/* Copyright (C) 2006 Free Software Foundation, Inc.
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

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#ifndef _BITS_SIGCONTEXT_H
#define _BITS_SIGCONTEXT_H 1

struct sigcontext {
  unsigned long sc_mask;
  unsigned long sc_usp;
  unsigned long sc_d0;
  unsigned long sc_d1;
#ifdef __mcoldfire__
  unsigned long sc_d2;
  unsigned long sc_d3;
  unsigned long sc_d4;
  unsigned long sc_d5;
  unsigned long sc_d6;
  unsigned long sc_d7;
#endif
  unsigned long sc_a0;
  unsigned long sc_a1;
#ifdef __mcoldfire__
  unsigned long sc_a2;
  unsigned long sc_a3;
  unsigned long sc_a4;
  unsigned long sc_a5;
  unsigned long sc_a6;
#endif
  unsigned short sc_sr;
  unsigned long sc_pc;
  unsigned short sc_formatvec;
#ifdef __mcoldfire__
  unsigned long sc_fpregs[8][2];
  unsigned long sc_fpcntl[3];
  unsigned char sc_fpstate[16];
#else
  unsigned long sc_fpregs[2*3];
  unsigned long sc_fpcntl[3];
  unsigned char sc_fpstate[216];
#endif
};

#endif
