#ifndef _MATH_PRIVATE_H

#define math_opt_barrier(x) \
({ __typeof(x) __x;					\
   if (sizeof (x) <= sizeof (double))			\
     __asm ("" : "=x" (__x) : "0" (x));			\
   else							\
     __asm ("" : "=t" (__x) : "0" (x));			\
   __x; })
#define math_force_eval(x) \
do							\
  {							\
    if (sizeof (x) <= sizeof (double))			\
      __asm __volatile ("" : : "x" (x));		\
    else						\
      __asm __volatile ("" : : "f" (x));		\
  }							\
while (0)

#include <math/math_private.h>

/* We can do a few things better on x86-64.  */

/* Direct movement of float into integer register.  */
#undef EXTRACT_WORDS64
#define EXTRACT_WORDS64(i,d)					\
do {								\
  long int i_;							\
  asm ("movd %1, %0" : "=rm" (i_) : "x" (d));			\
  (i) = i_;							\
} while (0)

/* And the reverse.  */
#undef INSERT_WORDS64
#define INSERT_WORDS64(d,i) \
do {								\
  long int i_ = i;						\
  asm ("movd %1, %0" : "=x" (d) : "rm" (i_));			\
} while (0)

/* Direct movement of float into integer register.  */
#undef GET_FLOAT_WORD
#define GET_FLOAT_WORD(i,d) \
do {								\
  int i_;							\
  asm ("movd %1, %0" : "=rm" (i_) : "x" (d));			\
  (i) = i_;							\
} while (0)

/* And the reverse.  */
#undef SET_FLOAT_WORD
#define SET_FLOAT_WORD(d,i) \
do {								\
  int i_ = i;							\
  asm ("movd %1, %0" : "=x" (d) : "rm" (i_));			\
} while (0)

#endif

#define __isnan(d) \
  ({ long int __di; EXTRACT_WORDS64 (__di, d);				      \
     (__di & 0x7fffffffffffffffl) > 0x7ff0000000000000l; })
#define __isnanf(d) \
  ({ int __di; GET_FLOAT_WORD (__di, d);				      \
     (__di & 0x7fffffff) > 0x7f800000; })

#define __isinf_ns(d) \
  ({ long int __di; EXTRACT_WORDS64 (__di, d);				      \
     (__di & 0x7fffffffffffffffl) == 0x7ff0000000000000l; })
#define __isinf_nsf(d) \
  ({ int __di; GET_FLOAT_WORD (__di, d);				      \
     (__di & 0x7fffffff) == 0x7f800000; })

#define __finite(d) \
  ({ long int __di; EXTRACT_WORDS64 (__di, d);				      \
     (__di & 0x7fffffffffffffffl) < 0x7ff0000000000000l; })
#define __finitef(d) \
  ({ int __di; GET_FLOAT_WORD (__di, d);				      \
     (__di & 0x7fffffff) < 0x7f800000; })
