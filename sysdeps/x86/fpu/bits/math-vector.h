/* Platform-specific SIMD declarations of math functions.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#ifndef _MATH_H
# error "Never include <bits/math-vector.h> directly;\
 include <math.h> instead."
#endif

/* Get default empty definitions for simd declarations.  */
#include <bits/libm-simd-decl-stubs.h>

#if defined __x86_64__ && defined __FAST_MATH__
# if defined _OPENMP && _OPENMP >= 201307
/* OpenMP case.  */
#  define __DECL_SIMD_x86_64 _Pragma ("omp declare simd notinbranch")
#  undef __DECL_SIMD_cos
#  define __DECL_SIMD_cos __DECL_SIMD_x86_64
#  undef __DECL_SIMD_cosf
#  define __DECL_SIMD_cosf __DECL_SIMD_x86_64
#  undef __DECL_SIMD_sin
#  define __DECL_SIMD_sin __DECL_SIMD_x86_64
#  undef __DECL_SIMD_sinf
#  define __DECL_SIMD_sinf __DECL_SIMD_x86_64
#  undef __DECL_SIMD_sincos
#  define __DECL_SIMD_sincos __DECL_SIMD_x86_64
#  undef __DECL_SIMD_sincosf
#  define __DECL_SIMD_sincosf __DECL_SIMD_x86_64
#  undef __DECL_SIMD_log
#  define __DECL_SIMD_log __DECL_SIMD_x86_64
#  undef __DECL_SIMD_logf
#  define __DECL_SIMD_logf __DECL_SIMD_x86_64
#  undef __DECL_SIMD_exp
#  define __DECL_SIMD_exp __DECL_SIMD_x86_64
#  undef __DECL_SIMD_expf
#  define __DECL_SIMD_expf __DECL_SIMD_x86_64
#  undef __DECL_SIMD_pow
#  define __DECL_SIMD_pow __DECL_SIMD_x86_64
#  undef __DECL_SIMD_powf
#  define __DECL_SIMD_powf __DECL_SIMD_x86_64

/* Workaround to exclude unnecessary symbol aliases in libmvec
   while GCC creates the vector names based on scalar asm name.
   Corresponding discussion started at
   <https://gcc.gnu.org/ml/gcc/2015-06/msg00173.html>.  */
__asm__ ("_ZGVbN2v___log_finite = _ZGVbN2v_log");
__asm__ ("_ZGVcN4v___log_finite = _ZGVcN4v_log");
__asm__ ("_ZGVdN4v___log_finite = _ZGVdN4v_log");
__asm__ ("_ZGVeN8v___log_finite = _ZGVeN8v_log");
__asm__ ("_ZGVbN4v___logf_finite = _ZGVbN4v_logf");
__asm__ ("_ZGVcN8v___logf_finite = _ZGVcN8v_logf");
__asm__ ("_ZGVdN8v___logf_finite = _ZGVdN8v_logf");
__asm__ ("_ZGVeN16v___logf_finite = _ZGVeN16v_logf");
__asm__ ("_ZGVbN2v___exp_finite = _ZGVbN2v_exp");
__asm__ ("_ZGVcN4v___exp_finite = _ZGVcN4v_exp");
__asm__ ("_ZGVdN4v___exp_finite = _ZGVdN4v_exp");
__asm__ ("_ZGVeN8v___exp_finite = _ZGVeN8v_exp");
__asm__ ("_ZGVbN4v___expf_finite = _ZGVbN4v_expf");
__asm__ ("_ZGVcN8v___expf_finite = _ZGVcN8v_expf");
__asm__ ("_ZGVdN8v___expf_finite = _ZGVdN8v_expf");
__asm__ ("_ZGVeN16v___expf_finite = _ZGVeN16v_expf");
__asm__ ("_ZGVbN2vv___pow_finite = _ZGVbN2vv_pow");
__asm__ ("_ZGVcN4vv___pow_finite = _ZGVcN4vv_pow");
__asm__ ("_ZGVdN4vv___pow_finite = _ZGVdN4vv_pow");
__asm__ ("_ZGVeN8vv___pow_finite = _ZGVeN8vv_pow");
__asm__ ("_ZGVbN4vv___powf_finite = _ZGVbN4vv_powf");
__asm__ ("_ZGVcN8vv___powf_finite = _ZGVcN8vv_powf");
__asm__ ("_ZGVdN8vv___powf_finite = _ZGVdN8vv_powf");
__asm__ ("_ZGVeN16vv___powf_finite = _ZGVeN16vv_powf");

# endif
#endif
