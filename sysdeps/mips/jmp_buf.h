/* Define the machine-dependent type `jmp_buf'.  Mips version.
   Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

typedef struct
  {
    /* Program counter.  */
    __ptr_t __pc;
    
    /* Stack pointer.  */
    __ptr_t __sp;
    
    /* Callee-saved registers s0 through s7.  */
    int __regs[8];
    
    /* The frame pointer.  */
    __ptr_t __fp;
    
    /* The global pointer.  */
    __ptr_t __gp;
    
    /* Floating point status register.  */
    int __fpc_csr;
    
    /* Callee-saved floating point registers.  */
    double __fpregs[6];
  } __jmp_buf[1];

#ifdef __USE_MISC
/* Offset to the program counter in `jmp_buf'.  */
#define JB_PC	0
#endif
