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
#include <stdlib.h>

#undef __longjmp

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

__NORETURN
void
DEFUN(__longjmp, (env, val_arg), CONST __jmp_buf env AND int val_arg)
{
  /* gcc 1.39.19 miscompiled the longjmp routine (as it did setjmp before
     the hack around it); force it to use $a1 for the longjmp value.
     Without this it saves $a1 in a register which gets clobbered
     along the way.  */
  register int val asm ("a1");

  /* Pull back the floating point callee-saved registers.  */
  asm volatile ("l.d $f20, %0" : : "m" (env[0].__fpregs[0]));
  asm volatile ("l.d $f22, %0" : : "m" (env[0].__fpregs[1]));
  asm volatile ("l.d $f24, %0" : : "m" (env[0].__fpregs[2]));
  asm volatile ("l.d $f26, %0" : : "m" (env[0].__fpregs[3]));
  asm volatile ("l.d $f28, %0" : : "m" (env[0].__fpregs[4]));
  asm volatile ("l.d $f30, %0" : : "m" (env[0].__fpregs[5]));
  
  /* Restore the stack pointer.  */
  asm volatile ("lw $29, %0" : : "m" (env[0].__sp));

  /* Get and reconstruct the floating point csr.  */
  asm volatile ("lw $2, %0" : : "m" (env[0].__fpc_csr));
  asm volatile ("ctc1 $2, $31");

  /* Get the FP.  */
  asm volatile ("lw $30, %0" : : "m" (env[0].__fp));

  /* Get the GP. */
  asm volatile ("lw $gp, %0" : : "m" (env[0].__gp));

  /* Get the callee-saved registers.  */
  asm volatile ("lw $16, %0" : : "m" (env[0].__regs[0]));
  asm volatile ("lw $17, %0" : : "m" (env[0].__regs[1]));
  asm volatile ("lw $18, %0" : : "m" (env[0].__regs[2]));
  asm volatile ("lw $19, %0" : : "m" (env[0].__regs[3]));
  asm volatile ("lw $20, %0" : : "m" (env[0].__regs[4]));
  asm volatile ("lw $21, %0" : : "m" (env[0].__regs[5]));
  asm volatile ("lw $22, %0" : : "m" (env[0].__regs[6]));
  asm volatile ("lw $23, %0" : : "m" (env[0].__regs[7]));

  /* Get the PC.  */
  asm volatile ("lw $31, %0" : : "m" (env[0].__pc));
  
  /* Give setjmp 1 if given a 0, or what they gave us if non-zero.  */
  if (val == 0)
    asm volatile ("li $2, 1");
  else
    asm volatile ("move $2, %0" : : "r" (val));

  asm volatile ("j $31");

  abort ();
}
