#ifndef X86_64_MATH_PRIVATE_H
#define X86_64_MATH_PRIVATE_H 1

/* We can do a few things better on x86-64.  */

#if defined __AVX__ || defined SSE2AVX
# define MOVD "vmovd"
#else
# define MOVD "movd"
#endif

/* Direct movement of float into integer register.  */
#define EXTRACT_WORDS64(i, d)						      \
  do {									      \
    int64_t i_;								      \
    asm (MOVD " %1, %0" : "=rm" (i_) : "x" ((double) (d)));		      \
    (i) = i_;								      \
  } while (0)

/* And the reverse.  */
#define INSERT_WORDS64(d, i) \
  do {									      \
    int64_t i_ = i;							      \
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

#include <sysdeps/i386/fpu/fenv_private.h>
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
