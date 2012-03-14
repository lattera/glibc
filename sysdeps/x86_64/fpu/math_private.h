#ifndef X86_64_MATH_PRIVATE_H
#define X86_64_MATH_PRIVATE_H 1

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

#include_next <math_private.h>

/* We can do a few things better on x86-64.  */

#if defined __AVX__ || defined SSE2AVX
# define MOVD "vmovd"
# define STMXCSR "vstmxcsr"
# define LDMXCSR "vldmxcsr"
#else
# define MOVD "movd"
# define STMXCSR "stmxcsr"
# define LDMXCSR "ldmxcsr"
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

extern __always_inline double
__ieee754_sqrt (double d)
{
  double res;
#if defined __AVX__ || defined SSE2AVX
  asm ("vsqrtsd %1, %0, %0" : "=x" (res) : "xm" (d));
#else
  asm ("sqrtsd %1, %0" : "=x" (res) : "xm" (d));
#endif
  return res;
}

extern __always_inline float
__ieee754_sqrtf (float d)
{
  float res;
#if defined __AVX__ || defined SSE2AVX
  asm ("vsqrtss %1, %0, %0" : "=x" (res) : "xm" (d));
#else
  asm ("sqrtss %1, %0" : "=x" (res) : "xm" (d));
#endif
  return res;
}

extern __always_inline long double
__ieee754_sqrtl (long double d)
{
  long double res;
  asm ("fsqrt" : "=t" (res) : "0" (d));
  return res;
}

#ifdef __SSE4_1__
extern __always_inline double
__rint (double d)
{
  double res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundsd $4, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundsd $4, %1, %0" : "=x" (res) : "xm" (d));
# endif
  return res;
}

extern __always_inline float
__rintf (float d)
{
  float res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundss $4, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundss $4, %1, %0" : "=x" (res) : "xm" (d));
# endif
  return res;
}

extern __always_inline double
__floor (double d)
{
  double res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundsd $1, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundsd $1, %1, %0" : "=x" (res) : "xm" (d));
# endif
  return res;
}

extern __always_inline float
__floorf (float d)
{
  float res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundss $1, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundss $1, %1, %0" : "=x" (res) : "xm" (d));
#  endif
  return res;
}
#endif /* __SSE4_1__ */


/* Specialized variants of the <fenv.h> interfaces which only handle
   either the FPU or the SSE unit.  */
#undef libc_feholdexcept
#define libc_feholdexcept(e) \
  do {									      \
     unsigned int mxcsr;						      \
     asm (STMXCSR " %0" : "=m" (*&mxcsr));				      \
     (e)->__mxcsr = mxcsr;						      \
     mxcsr = (mxcsr | 0x1f80) & ~0x3f;					      \
     asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));			      \
  } while (0)
#undef libc_feholdexceptf
#define libc_feholdexceptf(e) libc_feholdexcept (e)
// #define libc_feholdexceptl(e) (void) feholdexcept (e)

#undef libc_feholdexcept_setround
#define libc_feholdexcept_setround(e, r) \
  do {									      \
     unsigned int mxcsr;						      \
     asm (STMXCSR " %0" : "=m" (*&mxcsr));				      \
     (e)->__mxcsr = mxcsr;						      \
     mxcsr = ((mxcsr | 0x1f80) & ~0x603f) | ((r) << 3);			      \
     asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));			      \
  } while (0)
#undef libc_feholdexcept_setroundf
#define libc_feholdexcept_setroundf(e, r) libc_feholdexcept_setround (e, r)
// #define libc_feholdexcept_setroundl(e, r) ...

#undef libc_fetestexcept
#define libc_fetestexcept(e) \
  ({ unsigned int mxcsr;						      \
     asm volatile (STMXCSR " %0" : "=m" (*&mxcsr));			      \
     mxcsr & (e) & FE_ALL_EXCEPT; })
#undef libc_fetestexceptf
#define libc_fetestexceptf(e) libc_fetestexcept (e)
// #define libc_fetestexceptl(e) fetestexcept (e)

#undef libc_fesetenv
#define libc_fesetenv(e) \
  asm volatile (LDMXCSR " %0" : : "m" ((e)->__mxcsr))
#undef libc_fesetenvf
#define libc_fesetenvf(e) libc_fesetenv (e)
// #define libc_fesetenvl(e) (void) fesetenv (e)

#undef libc_feupdateenv
#define libc_feupdateenv(e) \
  do {									      \
    unsigned int mxcsr;							      \
    asm volatile (STMXCSR " %0" : "=m" (*&mxcsr));			      \
    asm volatile (LDMXCSR " %0" : : "m" ((e)->__mxcsr));		      \
    __feraiseexcept (mxcsr & FE_ALL_EXCEPT);				      \
  } while (0)
#undef libc_feupdateenvf
#define libc_feupdateenvf(e) libc_feupdateenv (e)
// #define libc_feupdateenvl(e) (void) feupdateenv (e)

#endif /* X86_64_MATH_PRIVATE_H */
