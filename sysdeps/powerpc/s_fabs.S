/* Floating-point absolute value.  PowerPC version.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

	.section ".text"
	.align 2
	.globl __fabs
	.type __fabs,@function
__fabs:	
/* double [f1] fabs (double [f1] x); */
	fabs %f1,%f1
	blr
0:
	.size	 __fabs,0b-__fabs

	.globl fabs,fabsf,__fabsf
	.weak fabs,fabsf
	.set fabs,__fabs
/* It turns out that it's safe to use this code even for single-precision.  */
	.set __fabsf,__fabs
	.set fabsf,__fabs
#ifdef NO_LONG_DOUBLE
	.globl fabsl,__fabsl
	.weak fabsl
	.set __fabsl,__fabs
	.set fabsl,__fabs
#endif
