/* Define the machine-dependent type `jmp_buf'.  Alpha version.
Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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
    /* Integer registers:
           $0 is the return value;
	   $1-$8, $22-$25, $28 are call-used;
	   $9-$14 we save here;
	   $15 is the FP and we save it here;
	   $16-$21 are input arguments (call-used);
	   $26 is the return PC and we save it here;
	   $27 is the procedure value (i.e., the address of __setjmp);
	   $29 is the global pointer, which the caller will reconstruct
	   from the return address restored in $26;
	   $30 is the stack pointer and we save it here;
	   $31 is always zero.  */
    long int __9, __10, __11, __12, __13, __14;
    long int *__pc, *__fp, *__sp;

#if 1				/* XXX need predefine for TARGET_FPREGS */
    /* Floating-point registers:
           $f0 is the floating return value;
	   $f1, $f10-$f15, $f22-$f30 are call-used;
	   $f2-$f9 we save here;
	   $f16-$21 are input args (call-used);
	   $f31 is always zero.  */
    double __f2, __f3, __f4, __f5, __f6, __f7, __f8, __f9;
#endif	/* Have FP regs.  */
  } __jmp_buf[1];
