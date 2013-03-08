#ifndef _MATH_PRIVATE_H_
#error "Never use <math_ldbl.h> directly; include <math_private.h> instead."
#endif

/* GCC does not optimize the default ldbl_pack code to not spill register
   in the stack. The following optimization tells gcc that pack/unpack
   is really a nop.  We use fr1/fr2 because those are the regs used to
   pass/return a single long double arg.  */
static inline long double
ldbl_pack_ppc (double a, double aa)
{
  register long double x __asm__ ("fr1");
  register double xh __asm__ ("fr1");
  register double xl __asm__ ("fr2");
  xh = a;
  xl = aa;
  __asm__ ("" : "=f" (x) : "f" (xh), "f" (xl));
  return x;
}

static inline void
ldbl_unpack_ppc (long double l, double *a, double *aa)
{
  register long double x __asm__ ("fr1");
  register double xh __asm__ ("fr1");
  register double xl __asm__ ("fr2");
  x = l;
  __asm__ ("" : "=f" (xh), "=f" (xl) : "f" (x));
  *a = xh;
  *aa = xl;
}

#define ldbl_pack   ldbl_pack_ppc
#define ldbl_unpack ldbl_unpack_ppc

#include <sysdeps/ieee754/ldbl-128ibm/math_ldbl.h>
