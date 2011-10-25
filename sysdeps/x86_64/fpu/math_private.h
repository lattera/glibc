#ifndef _MATH_PRIVATE_H

#define math_opt_barrier(x) \
  ({ __typeof(x) __x;							      \
     if (sizeof (x) <= sizeof (double))					      \
       __asm ("" : "=x" (__x) : "0" (x));				      \
     else								      \
       __asm ("" : "=t" (__x) : "0" (x));				      \
     __x; })
#define math_force_eval(x) \
  do {									      \
    if (sizeof (x) <= sizeof (double))					      \
      __asm __volatile ("" : : "x" (x));				      \
    else								      \
      __asm __volatile ("" : : "f" (x));				      \
  } while (0)

#include <math/math_private.h>

/* We can do a few things better on x86-64.  */

#ifdef __AVX__
# define MOVD "vmovd"
#else
# define MOVD "movd"
#endif

/* Direct movement of float into integer register.  */
#undef EXTRACT_WORDS64
#define EXTRACT_WORDS64(i, d)						      \
  do {									      \
    long int i_;							      \
    asm (MOVD " %1, %0" : "=rm" (i_) : "x" ((double) (d)));		      \
    (i) = i_;								      \
  } while (0)

/* And the reverse.  */
#undef INSERT_WORDS64
#define INSERT_WORDS64(d, i) \
  do {									      \
    long int i_ = i;							      \
    double d__;								      \
    asm (MOVD " %1, %0" : "=x" (d__) : "rm" (i_));			      \
    d = d__;								      \
  } while (0)

/* Direct movement of float into integer register.  */
#undef GET_FLOAT_WORD
#define GET_FLOAT_WORD(i, d) \
  do {									      \
    int i_;								      \
    asm (MOVD " %1, %0" : "=rm" (i_) : "x" ((float) (d)));		      \
    (i) = i_;								      \
  } while (0)

/* And the reverse.  */
#undef SET_FLOAT_WORD
#define SET_FLOAT_WORD(f, i) \
  do {									      \
    int i_ = i;								      \
    float f__;								      \
    asm (MOVD " %1, %0" : "=x" (f__) : "rm" (i_));			      \
    f = f__;								      \
  } while (0)

#endif

#define __isnan(d) \
  ({ long int __di; EXTRACT_WORDS64 (__di, (double) (d));		      \
     (__di & 0x7fffffffffffffffl) > 0x7ff0000000000000l; })
#define __isnanf(d) \
  ({ int __di; GET_FLOAT_WORD (__di, (float) d);			      \
     (__di & 0x7fffffff) > 0x7f800000; })

#define __isinf_ns(d) \
  ({ long int __di; EXTRACT_WORDS64 (__di, (double) (d));		      \
     (__di & 0x7fffffffffffffffl) == 0x7ff0000000000000l; })
#define __isinf_nsf(d) \
  ({ int __di; GET_FLOAT_WORD (__di, (float) d);			      \
     (__di & 0x7fffffff) == 0x7f800000; })

#define __finite(d) \
  ({ long int __di; EXTRACT_WORDS64 (__di, (double) (d));		      \
     (__di & 0x7fffffffffffffffl) < 0x7ff0000000000000l; })
#define __finitef(d) \
  ({ int __di; GET_FLOAT_WORD (__di, (float) d);			      \
     (__di & 0x7fffffff) < 0x7f800000; })

#ifdef __AVX__
# define __ieee754_sqrt(d) \
  ({ double __res;							      \
    asm ("vsqrtsd %1, %0, %0" : "=x" (__res) : "xm" ((double) (d)));	      \
     __res; })
# define __ieee754_sqrtf(d) \
  ({ float __res;							      \
    asm ("vsqrtss %1, %0, %0" : "=x" (__res) : "xm" ((float) (d)));	      \
     __res; })
#else
# define __ieee754_sqrt(d) \
  ({ double __res;							      \
    asm ("sqrtsd %1, %0" : "=x" (__res) : "xm" ((double) (d)));		      \
     __res; })
# define __ieee754_sqrtf(d) \
  ({ float __res;							      \
    asm ("sqrtss %1, %0" : "=x" (__res) : "xm" ((float) (d)));		      \
     __res; })
#endif
#define __ieee754_sqrtl(d) \
  ({ long double __res;							      \
    asm ("fsqrt" : "=t" (__res) : "0" ((long double) (d)));		      \
     __res; })

#ifdef __SSE4_1__
# ifndef __rint
#  ifdef __AVX__
#   define __rint(d) \
  ({ double __res; \
    asm ("vroundsd $4, %1, %0, %0" : "=x" (__res) : "xm" ((double) (d)));      \
     __res; })
#  else
#   define __rint(d) \
  ({ double __res; \
    asm ("roundsd $4, %1, %0" : "=x" (__res) : "xm" ((double) (d)));	      \
     __res; })
#  endif
# endif
# ifndef __rintf
#  ifdef __AVX__
#   define __rintf(d) \
  ({ float __res; \
    asm ("vroundss $4, %1, %0, %0" : "=x" (__res) : "xm" ((float) (d)));      \
     __res; })
#  else
#   define __rintf(d) \
  ({ float __res; \
    asm ("roundss $4, %1, %0" : "=x" (__res) : "xm" ((float) (d)));	      \
     __res; })
#  endif
# endif

# ifndef __floor
#  ifdef __AVX__
#   define __floor(d) \
  ({ double __res; \
    asm ("vroundsd $1, %1, %0, %0" : "=x" (__res) : "xm" ((double) (d)));      \
     __res; })
#  else
#   define __floor(d) \
  ({ double __res; \
    asm ("roundsd $1, %1, %0" : "=x" (__res) : "xm" ((double) (d)));	      \
     __res; })
#  endif
# endif
# ifndef __floorf
#  ifdef __AVX__
#   define __floorf(d) \
  ({ float __res; \
    asm ("vroundss $1, %1, %0, %0" : "=x" (__res) : "xm" ((float) (d)));      \
     __res; })
#  else
#   define __floorf(d) \
  ({ float __res; \
    asm ("roundss $1, %1, %0" : "=x" (__res) : "xm" ((float) (d)));	      \
     __res; })
#  endif
# endif
#endif


/* Specialized variants of the <fenv.h> interfaces which only handle
   either the FPU or the SSE unit.  */
#undef libc_fegetround
#define libc_fegetround() \
  ({									      \
     unsigned int mxcsr;						      \
     asm volatile ("stmxcsr %0" : "=m" (*&mxcsr));			      \
     (mxcsr & 0x6000) >> 3;						      \
  })
#undef libc_fegetroundf
#define libc_fegetroundf() libc_fegetround ()
// #define libc_fegetroundl() fegetround ()

#undef libc_fesetround
#define libc_fesetround(r) \
  do {									      \
     unsigned int mxcsr;						      \
     asm ("stmxcsr %0" : "=m" (*&mxcsr));				      \
     mxcsr = (mxcsr & ~0x6000) | ((r) << 3);				      \
     asm volatile ("ldmxcsr %0" : : "m" (*&mxcsr));			      \
  } while (0)
#undef libc_fesetroundf
#define libc_fesetroundf(r) libc_fesetround (r)
// #define libc_fesetroundl(r) (void) fesetround (r)

#undef libc_feholdexcept
#ifdef __AVX__
# define libc_feholdexcept(e) \
  do {									      \
     unsigned int mxcsr;						      \
     asm ("vstmxcsr %0" : "=m" (*&mxcsr));				      \
     (e)->__mxcsr = mxcsr;						      \
     mxcsr = (mxcsr | 0x1f80) & ~0x3f;					      \
     asm volatile ("vldmxcsr %0" : : "m" (*&mxcsr));			      \
  } while (0)
#else
# define libc_feholdexcept(e) \
  do {									      \
     unsigned int mxcsr;						      \
     asm ("stmxcsr %0" : "=m" (*&mxcsr));				      \
     (e)->__mxcsr = mxcsr;						      \
     mxcsr = (mxcsr | 0x1f80) & ~0x3f;					      \
     asm volatile ("ldmxcsr %0" : : "m" (*&mxcsr));			      \
  } while (0)
#endif
#undef libc_feholdexceptf
#define libc_feholdexceptf(e) libc_feholdexcept (e)
// #define libc_feholdexceptl(e) (void) feholdexcept (e)

#undef libc_feholdexcept_setround
#ifdef __AVX__
# define libc_feholdexcept_setround(e, r) \
  do {									      \
     unsigned int mxcsr;						      \
     asm ("vstmxcsr %0" : "=m" (*&mxcsr));				      \
     (e)->__mxcsr = mxcsr;						      \
     mxcsr = ((mxcsr | 0x1f80) & ~0x603f) | ((r) << 3);			      \
     asm volatile ("vldmxcsr %0" : : "m" (*&mxcsr));			      \
  } while (0)
#else
# define libc_feholdexcept_setround(e, r) \
  do {									      \
     unsigned int mxcsr;						      \
     asm ("stmxcsr %0" : "=m" (*&mxcsr));				      \
     (e)->__mxcsr = mxcsr;						      \
     mxcsr = ((mxcsr | 0x1f80) & ~0x603f) | ((r) << 3);			      \
     asm volatile ("ldmxcsr %0" : : "m" (*&mxcsr));			      \
  } while (0)
#endif
#undef libc_feholdexcept_setroundf
#define libc_feholdexcept_setroundf(e, r) libc_feholdexcept_setround (e, r)
// #define libc_feholdexcept_setroundl(e, r) ...

#undef libc_fetestexcept
#ifdef __AVX__
# define libc_fetestexcept(e) \
  ({ unsigned int mxcsr; asm volatile ("vstmxcsr %0" : "=m" (*&mxcsr));	      \
     mxcsr & (e) & FE_ALL_EXCEPT; })
#else
# define libc_fetestexcept(e) \
  ({ unsigned int mxcsr; asm volatile ("stmxcsr %0" : "=m" (*&mxcsr));	      \
     mxcsr & (e) & FE_ALL_EXCEPT; })
#endif
#undef libc_fetestexceptf
#define libc_fetestexceptf(e) libc_fetestexcept (e)
// #define libc_fetestexceptl(e) fetestexcept (e)

#undef libc_fesetenv
#ifdef __AVX__
# define libc_fesetenv(e) \
  asm volatile ("vldmxcsr %0" : : "m" ((e)->__mxcsr))
#else
# define libc_fesetenv(e) \
  asm volatile ("ldmxcsr %0" : : "m" ((e)->__mxcsr))
#endif
#undef libc_fesetenvf
#define libc_fesetenvf(e) libc_fesetenv (e)
// #define libc_fesetenvl(e) (void) fesetenv (e)

#undef libc_feupdateenv
#ifdef __AVX__
# define libc_feupdateenv(e) \
  do {									      \
    unsigned int mxcsr;							      \
    asm volatile ("vstmxcsr %0" : "=m" (*&mxcsr));			      \
    asm volatile ("vldmxcsr %0" : : "m" ((e)->__mxcsr));		      \
    __feraiseexcept (mxcsr & FE_ALL_EXCEPT);				      \
  } while (0)
#else
# define libc_feupdateenv(e) \
  do {									      \
    unsigned int mxcsr;							      \
    asm volatile ("stmxcsr %0" : "=m" (*&mxcsr));			      \
    asm volatile ("ldmxcsr %0" : : "m" ((e)->__mxcsr));			      \
    __feraiseexcept (mxcsr & FE_ALL_EXCEPT);				      \
  } while (0)
#endif
#undef libc_feupdateenvf
#define libc_feupdateenvf(e) libc_feupdateenv (e)
// #define libc_feupdateenvl(e) (void) feupdateenv (e)
