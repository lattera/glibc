/* Copyright (C) 1992 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@cs.widener.edu).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* Define the machine-dependent type `jmp_buf'.  Mips version.  */

typedef struct
{
  /* Program counter when the signal hit.  */
  int __pc;

  /* Stack pointer.  */
  int __sp;

  /* Callee-saved registers s0 through s7.  */
  int __regs[8];

  /* The frame pointer.  */
  int __fp;

  /* The global pointer.  */
  int __gp;

  /* Floating point status register.  */
  int __fpc_csr;

  /* Callee-saved floating point registers.  */
  int __fpregs[6];

} __jmp_buf[1];
