/* Copyright (C) 1997-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H  1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#include <bits/wordsize.h>

#if __WORDSIZE == 64

#ifndef _ASM
typedef struct __sparc64_jmp_buf
  {
    struct __sparc64_jmp_buf	*uc_link;
    unsigned long		uc_flags;
    unsigned long		uc_sigmask;
    struct __sparc64_jmp_buf_mcontext
      {
	unsigned long		mc_gregs[19];
	unsigned long		mc_fp;
	unsigned long		mc_i7;
	struct __sparc64_jmp_buf_fpu
	  {
	    union
	      {
		unsigned int	sregs[32];
		unsigned long	dregs[32];
		long double	qregs[16];
	      }			mcfpu_fpregs;
	    unsigned long	mcfpu_fprs;
	    unsigned long	mcfpu_gsr;
	    void		*mcfpu_fq;
	    unsigned char	mcfpu_qcnt;
	    unsigned char	mcfpu_qentsz;
	    unsigned char	mcfpu_enab;
	  }			mc_fpregs;
      }				uc_mcontext;
  } __jmp_buf[1];
#endif

#else

#ifndef _ASM
typedef int __jmp_buf[3];
#endif

#endif

#endif  /* bits/setjmp.h */
