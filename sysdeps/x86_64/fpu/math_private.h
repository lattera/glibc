#ifndef X86_64_MATH_PRIVATE_H
#define X86_64_MATH_PRIVATE_H 1

#include <fenv.h>

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
#define EXTRACT_WORDS64(i, d)						      \
  do {									      \
    long int i_;							      \
    asm (MOVD " %1, %0" : "=rm" (i_) : "x" ((double) (d)));		      \
    (i) = i_;								      \
  } while (0)

/* And the reverse.  */
#define INSERT_WORDS64(d, i) \
  do {									      \
    long int i_ = i;							      \
    double d__;								      \
    asm (MOVD " %1, %0" : "=x" (d__) : "rm" (i_));			      \
    d = d__;								      \
  } while (0)

/* Direct movement of float into integer register.  */
#define GET_FLOAT_WORD(i, d) \
  do {									      \
    int i_;								      \
    asm (MOVD " %1, %0" : "=rm" (i_) : "x" ((float) (d)));		      \
    (i) = i_;								      \
  } while (0)

/* And the reverse.  */
#define SET_FLOAT_WORD(f, i) \
  do {									      \
    int i_ = i;								      \
    float f__;								      \
    asm (MOVD " %1, %0" : "=x" (f__) : "rm" (i_));			      \
    f = f__;								      \
  } while (0)

/* Specialized variants of the <fenv.h> interfaces which only handle
   either the FPU or the SSE unit.  */
static __always_inline void
libc_feholdexcept (fenv_t *e)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  e->__mxcsr = mxcsr;
  mxcsr = (mxcsr | 0x1f80) & ~0x3f;
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}
#define libc_feholdexcept  libc_feholdexcept
#define libc_feholdexceptf libc_feholdexcept

static __always_inline void
libc_feholdexcept_setround (fenv_t *e, int r)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  e->__mxcsr = mxcsr;
  mxcsr = ((mxcsr | 0x1f80) & ~0x603f) | (r << 3);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}
#define libc_feholdexcept_setround  libc_feholdexcept_setround
#define libc_feholdexcept_setroundf libc_feholdexcept_setround

static __always_inline int
libc_fetestexcept (int e)
{
  unsigned int mxcsr;
  asm volatile (STMXCSR " %0" : "=m" (*&mxcsr));
  return mxcsr & e & FE_ALL_EXCEPT;
}
#define libc_fetestexcept  libc_fetestexcept
#define libc_fetestexceptf libc_fetestexcept

static __always_inline void
libc_fesetenv (fenv_t *e)
{
  asm volatile (LDMXCSR " %0" : : "m" (e->__mxcsr));
}
#define libc_fesetenv  libc_fesetenv
#define libc_fesetenvf libc_fesetenv

static __always_inline int
libc_feupdateenv_test (fenv_t *e, int ex)
{
  unsigned int mxcsr, old_mxcsr, cur_ex;
  asm volatile (STMXCSR " %0" : "=m" (*&mxcsr));
  cur_ex = mxcsr & FE_ALL_EXCEPT;

  /* Merge current exceptions with the old environment.  */
  old_mxcsr = e->__mxcsr;
  mxcsr = old_mxcsr | cur_ex;
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));

  /* Raise SIGFPE for any new exceptions since the hold.  Expect that
     the normal environment has all exceptions masked.  */
  if (__builtin_expect ((old_mxcsr >> 7) & cur_ex, 0))
    __feraiseexcept (cur_ex);

  /* Test for exceptions raised since the hold.  */
  return cur_ex & ex;
}
#define libc_feupdateenv_test  libc_feupdateenv_test
#define libc_feupdateenv_testf libc_feupdateenv_test

static __always_inline void
libc_feupdateenv (fenv_t *e)
{
  libc_feupdateenv_test (e, 0);
}
#define libc_feupdateenv  libc_feupdateenv
#define libc_feupdateenvf libc_feupdateenv

static __always_inline void
libc_feholdsetround (fenv_t *e, int r)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  e->__mxcsr = mxcsr;
  mxcsr = (mxcsr & ~0x6000) | (r << 3);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}
#define libc_feholdsetround  libc_feholdsetround
#define libc_feholdsetroundf libc_feholdsetround

static __always_inline void
libc_feresetround (fenv_t *e)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  mxcsr = (mxcsr & ~0x6000) | (e->__mxcsr & 0x6000);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}
#define libc_feresetround  libc_feresetround
#define libc_feresetroundf libc_feresetround

#include_next <math_private.h>

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

#endif /* X86_64_MATH_PRIVATE_H */
