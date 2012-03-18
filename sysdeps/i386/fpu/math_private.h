#ifndef _MATH_PRIVATE_H

#include <fenv.h>
#include <fpu_control.h>

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

static __always_inline void
libc_feholdexcept_setround_53bit (fenv_t *e, int r)
{
  feholdexcept (e);
  fesetround (r);

  fpu_control_t cw;
  _FPU_GETCW (cw);
  cw &= ~(fpu_control_t) _FPU_EXTENDED;
  cw |= _FPU_DOUBLE;
  _FPU_SETCW (cw);
}
#define libc_feholdexcept_setround_53bit libc_feholdexcept_setround_53bit

static __always_inline void
libc_feupdateenv_53bit (fenv_t *e)
{
  feupdateenv (e);

  /* Unfortunately, feupdateenv fails to affect the rounding precision.
     We can get that back by restoring the exact control word we saved.  */
  _FPU_SETCW (e->__control_word);
}
#define libc_feupdateenv_53bit libc_feupdateenv_53bit

#include_next <math_private.h>

#endif /* _MATH_PRIVATE_H */
