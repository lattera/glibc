#ifndef	_MATH_H

#include <math/math.h>

/* Now define the internal interfaces.  */
extern int __matherr (struct exception *__exc);

extern int __finite_internal (double __value)
     __attribute__ ((__const__)) attribute_hidden;
extern int __finitef_internal (float __value)
     __attribute__ ((__const__)) attribute_hidden;

extern int __isinf_internal (double __value)
     __attribute__ ((__const__)) attribute_hidden;
extern int __isnan_internal (double __value)
     __attribute__ ((__const__)) attribute_hidden;

#ifndef __NO_LONG_DOUBLE_MATH
extern int __finitel_internal (long double __value)
     __attribute__ ((__const__)) attribute_hidden;

extern int __isinfl_internal (long double __value)
     __attribute__ ((__const__)) attribute_hidden;
extern int __isnanl_internal (long double __value)
     __attribute__ ((__const__)) attribute_hidden;
#endif

#if !defined NOT_IN_libc || defined IS_IN_libm
# undef isfinite
# ifdef __NO_LONG_DOUBLE_MATH
#  define isfinite(x) \
     (sizeof (x) == sizeof (float)					      \
      ? INTUSE(__finitef) (x) : INTUSE(__finite) (x))
# else
#  define isfinite(x) \
     (sizeof (x) == sizeof (float)					      \
      ? INTUSE(__finitef) (x)						      \
      : sizeof (x) == sizeof (double)					      \
      ? INTUSE(__finite) (x) : INTUSE(__finitel) (x))
# endif
#endif


#endif
