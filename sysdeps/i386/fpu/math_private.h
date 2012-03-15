#ifndef _MATH_PRIVATE_H

#define math_opt_barrier(x) \
({ __typeof (x) __x;					\
   __asm ("" : "=t" (__x) : "0" (x));			\
   __x; })
#define math_force_eval(x) \
do							\
  {							\
    __typeof (x) __x = (x);				\
    if (sizeof (x) <= sizeof (double))			\
      __asm __volatile ("" : : "m" (__x));		\
    else						\
      __asm __volatile ("" : : "f" (__x));		\
  }							\
while (0)

#include_next <math_private.h>

#include <fpu_control.h>

#undef libc_feholdexcept_setround_53bit
#define libc_feholdexcept_setround_53bit(e, r)	\
  do						\
    {						\
      fpu_control_t cw;				\
      libc_feholdexcept_setround (e, r);	\
      _FPU_GETCW (cw);				\
      cw &= ~(fpu_control_t) _FPU_EXTENDED;	\
      cw |= _FPU_DOUBLE;			\
      _FPU_SETCW (cw);				\
    }						\
  while (0)

#undef libc_feupdateenv_53bit
#define libc_feupdateenv_53bit(e)		\
  do						\
    {						\
      fpu_control_t cw;				\
      libc_feupdateenv (e);			\
      _FPU_GETCW (cw);				\
      cw &= ~(fpu_control_t) _FPU_EXTENDED;	\
      cw |= _FPU_EXTENDED;			\
      _FPU_SETCW (cw);				\
    }						\
  while (0)

#endif
