/* Copyright (C) 1992 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <ansidecl.h>
#include <setjmp.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* This function is only called via the assembly language routine
   __setjmp, which arranges to pass in the stack pointer and the frame
   pointer.  We do things this way because it's difficult to reliably
   access them in C.  */

int
DEFUN(__setjmp_aux, (env, sp, fp), __jmp_buf env AND int sp AND int fp)
{
  /* Store the floating point callee-saved registers...  */
  asm volatile ("s.d $f20, %0" : : "m" (env[0].__fpregs[0]));
  asm volatile ("s.d $f22, %0" : : "m" (env[0].__fpregs[1]));
  asm volatile ("s.d $f24, %0" : : "m" (env[0].__fpregs[2]));
  asm volatile ("s.d $f26, %0" : : "m" (env[0].__fpregs[3]));
  asm volatile ("s.d $f28, %0" : : "m" (env[0].__fpregs[4]));
  asm volatile ("s.d $f30, %0" : : "m" (env[0].__fpregs[5]));

  /* .. and the PC;  */
  asm volatile ("sw $31, %0" : : "m" (env[0].__pc));

  /* .. and the stack pointer;  */
  asm volatile ("sw %1, %0" : : "m" (env[0].__sp), "r" (sp));

  /* .. and the FP; it'll be in s8. */
  asm volatile ("sw %1, %0" : : "m" (env[0].__fp), "r" (fp));

  /* .. and the GP; */
  asm volatile ("sw $gp, %0" : : "m" (env[0].__gp));

  /* .. and the callee-saved registers; */
  asm volatile ("sw $16, %0" : : "m" (env[0].__regs[0]));
  asm volatile ("sw $17, %0" : : "m" (env[0].__regs[1]));
  asm volatile ("sw $18, %0" : : "m" (env[0].__regs[2]));
  asm volatile ("sw $19, %0" : : "m" (env[0].__regs[3]));
  asm volatile ("sw $20, %0" : : "m" (env[0].__regs[4]));
  asm volatile ("sw $21, %0" : : "m" (env[0].__regs[5]));
  asm volatile ("sw $22, %0" : : "m" (env[0].__regs[6]));
  asm volatile ("sw $23, %0" : : "m" (env[0].__regs[7]));

  /* .. and finally get and reconstruct the floating point csr.  */
  asm volatile ("cfc1 $2, $31");
  asm volatile ("sw $2, %0" : : "m" (env[0].__fpc_csr));

  return 0;
}
