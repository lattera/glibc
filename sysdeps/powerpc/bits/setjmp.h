/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  PowerPC version.  */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* The previous bits/setjmp.h had __jmp_buf defined as a structure.
   We use an array of 'long int' instead, to make writing the
   assembler easier. Naturally, user code should not depend on
   either representation. */

#if defined __USE_MISC || defined _ASM
# define JB_GPR1   0   /* also known as the stack pointer */
# define JB_GPR2   1
# define JB_LR     2
# define JB_GPRS   3  /* GPRs 14 through 31 are saved, 18 in total */
# define JB_UNUSED 21 /* it's sometimes faster to store doubles word-aligned */
# define JB_FPRS   22 /* FPRs 14 through 31 are saved, 18*2 words total */
#endif

#ifndef	_ASM
typedef long int __jmp_buf[58];
#endif
