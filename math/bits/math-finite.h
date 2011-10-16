/* Entry points to finite-math-only compiler runs.
   Copyright (C) 2011 Free Software Foundation, Inc.
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

#ifndef _MATH_H
# error "Never use <bits/math-finite.h> directly; include <math.h> instead."
#endif

/* acos.  */
extern double acos (double) __asm__ ("__acos_finite");
extern float acosf (float) __asm__ ("__acosf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double acosl (long double) __asm__ ("__acosl_finite");
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
/* acosh.  */
extern double acosh (double) __asm__ ("__acosh_finite");
extern float acoshf (float) __asm__ ("__acoshf_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double acoshl (long double) __asm__ ("__acoshl_finite");
# endif
#endif

/* asin.  */
extern double asin (double) __asm__ ("__asin_finite");
extern float asinf (float) __asm__ ("__asinf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double asinl (long double) __asm__ ("__asinl_finite");
#endif

/* atan2.  */
extern double atan2 (double, double) __asm__ ("__atan2_finite");
extern float atan2f (float, float) __asm__ ("__atan2f_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double atan2l (long double, long double) __asm__ ("__atan2l_finite");
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
/* atanh.  */
extern double atanh (double) __asm__ ("__atanh_finite");
extern float atanhf (float) __asm__ ("__atanhf_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double atanhl (long double) __asm__ ("__atanhl_finite");
# endif
#endif

/* cosh.  */
extern double cosh (double) __asm__ ("__cosh_finite");
extern float coshf (float) __asm__ ("__coshf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double coshl (long double) __asm__ ("__coshl_finite");
#endif

/* exp.  */
extern double exp (double) __asm__ ("__exp_finite");
extern float expf (float) __asm__ ("__expf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double expl (long double) __asm__ ("__expl_finite");
#endif

#ifdef __USE_GNU
/* exp10.  */
extern double exp10 (double) __asm__ ("__exp10_finite");
extern float exp10f (float) __asm__ ("__exp10f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double exp10l (long double) __asm__ ("__exp10l_finite");
# endif

/* pow10.  */
extern double pow10 (double) __asm__ ("__exp10_finite");
extern float pow10f (float) __asm__ ("__exp10f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double pow10l (long double) __asm__ ("__exp10l_finite");
# endif
#endif

#ifdef __USE_ISOC99
/* exp2.  */
extern double exp2 (double) __asm__ ("__exp2_finite");
extern float exp2f (float) __asm__ ("__exp2f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double exp2l (long double) __asm__ ("__exp2l_finite");
# endif
#endif

/* fmod.  */
extern double fmod (double, double) __asm__ ("__fmod_finite");
extern float fmodf (float, float) __asm__ ("__fmodf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double fmodl (long double, long double) __asm__ ("__fmodl_finite");
#endif

#ifdef __USE_ISOC99
/* hypot.  */
extern double hypot (double, double) __asm__ ("__hypot_finite");
extern float hypotf (float, float) __asm__ ("__hypotf_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double hypotl (long double, long double) __asm__ ("__hypotl_finite");
# endif
#endif

#if defined __USE_MISC || defined __USE_XOPEN
/* j0.  */
extern double j0 (double) __asm__ ("__j0_finite");
extern float j0f (float) __asm__ ("__j0f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double j0l (long double) __asm__ ("__j0l_finite");
# endif

/* y0.  */
extern double y0 (double) __asm__ ("__y0_finite");
extern float y0f (float) __asm__ ("__y0f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double y0l (long double) __asm__ ("__y0l_finite");
# endif

/* j1.  */
extern double j1 (double) __asm__ ("__j1_finite");
extern float j1f (float) __asm__ ("__j1f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double j1l (long double) __asm__ ("__j1l_finite");
# endif

/* y1.  */
extern double y1 (double) __asm__ ("__y1_finite");
extern float y1f (float) __asm__ ("__y1f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double y1l (long double) __asm__ ("__y1l_finite");
# endif

/* jn.  */
extern double jn (int, double) __asm__ ("__jn_finite");
extern float jnf (int, float) __asm__ ("__jnf_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double jnl (int, long double) __asm__ ("__jnl_finite");
# endif

/* yn.  */
extern double yn (int, double) __asm__ ("__yn_finite");
extern float ynf (int, float) __asm__ ("__ynf_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double ynl (int, long double) __asm__ ("__ynl_finite");
# endif
#endif

#ifdef __USE_MISC
/* lgamma_r.  */
extern double lgamma_r (double, int *) __asm__ ("__lgamma_r_finite");
extern float lgammaf_r (float, int *) __asm__ ("__lgammaf_r_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double lgammal_r (long double, int *) __asm__ ("__lgammal_r_finite");
# endif
#endif

#if defined __USE_MISC || defined __USE_XOPEN || defined __USE_ISOC99
/* lgamma.  */
__extern_always_inline double lgamma (double __d)
{
# ifdef __USE_ISOC99
  int __local_signgam = 0;
  return lgamma_r (__d, &__local_signgam);
# else
  return lgamma_r (__d, &signgam);
# endif
}
__extern_always_inline float lgammaf (float __d)
{
# ifdef __USE_ISOC99
  int __local_signgam = 0;
  return lgammaf_r (__d, &__local_signgam);
# else
  return lgammaf_r (__d, &signgam);
# endif
}
# ifdef __MATH_DECLARE_LDOUBLE
__extern_always_inline long double lgammal (long double __d)
{
# ifdef __USE_ISOC99
  int __local_signgam = 0;
  return lgammal_r (__d, &__local_signgam);
# else
  return lgammal_r (__d, &signgam);
# endif
}
# endif
#endif

#if defined __USE_MISC || defined __USE_XOPEN
/* gamma.  */
__extern_always_inline double gamma (double __d)
{
# ifdef __USE_ISOC99
  int __local_signgam = 0;
  return lgamma_r (__d, &__local_signgam);
# else
  return lgamma_r (__d, &signgam);
# endif
}
__extern_always_inline float gammaf (float __d)
{
# ifdef __USE_ISOC99
  int __local_signgam = 0;
  return lgammaf_r (__d, &__local_signgam);
# else
  return lgammaf_r (__d, &signgam);
# endif
}
# ifdef __MATH_DECLARE_LDOUBLE
__extern_always_inline long double gammal (long double __d)
{
#  ifdef __USE_ISOC99
  int __local_signgam = 0;
  return lgammal_r (__d, &__local_signgam);
#  else
  return lgammal_r (__d, &signgam);
#  endif
}
# endif
#endif

/* log.  */
extern double log (double) __asm__ ("__log_finite");
extern float logf (float) __asm__ ("__logf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double logl (long double) __asm__ ("__logl_finite");
#endif

/* log10.  */
extern double log10 (double) __asm__ ("__log10_finite");
extern float log10f (float) __asm__ ("__log10f_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double log10l (long double) __asm__ ("__log10l_finite");
#endif

#ifdef __USE_ISOC99
/* log2.  */
extern double log2 (double) __asm__ ("__log2_finite");
extern float log2f (float) __asm__ ("__log2f_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double log2l (long double) __asm__ ("__log2l_finite");
# endif
#endif

/* pow.  */
extern double pow (double, double) __asm__ ("__pow_finite");
extern float powf (float, float) __asm__ ("__powf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double powl (long double, long double) __asm__ ("__powl_finite");
#endif

/* remainder.  */
extern double remainder (double, double) __asm__ ("__remainder_finite");
extern float remainderf (float, float) __asm__ ("__remainderf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double remainderl (long double, long double) __asm__ ("__remainderl_finite");
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED
/* scalb.  */
extern double scalb (double, double) __asm__ ("__scalb_finite");
extern float scalbf (float, float) __asm__ ("__scalbf_finite");
# ifdef __MATH_DECLARE_LDOUBLE
extern long double scalbl (long double, long double) __asm__ ("__scalbl_finite");
# endif
#endif

/* sinh.  */
extern double sinh (double) __asm__ ("__sinh_finite");
extern float sinhf (float) __asm__ ("__sinhf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double sinhl (long double) __asm__ ("__sinhl_finite");
#endif

/* sqrt.  */
extern double sqrt (double) __asm__ ("__sqrt_finite");
extern float sqrtf (float) __asm__ ("__sqrtf_finite");
#ifdef __MATH_DECLARE_LDOUBLE
extern long double sqrtl (long double) __asm__ ("__sqrtl_finite");
#endif

#ifdef __USE_ISOC99
/* tgamma.  */
extern double __gamma_r_finite (double, int *);
__extern_always_inline double tgamma (double __d)
{
  int __local_signgam = 0;
  double __res = __gamma_r_finite (__d, &__local_signgam);
  return __local_signgam < 0 ? -__res : __res;
}
extern float __gammaf_r_finite (float, int *);
__extern_always_inline float tgammaf (float __d)
{
  int __local_signgam = 0;
  float __res = __gammaf_r_finite (__d, &__local_signgam);
  return __local_signgam < 0 ? -__res : __res;
}
# ifdef __MATH_DECLARE_LDOUBLE
extern long double __gammal_r_finite (long double, int *);
__extern_always_inline long double tgammal (long double __d)
{
  int __local_signgam = 0;
  long double __res = __gammal_r_finite (__d, &__local_signgam);
  return __local_signgam < 0 ? -__res : __res;
}
# endif
#endif
