#ifndef	_MATH_H

#include <math/math.h>

/* Now define the internal interfaces.  */
extern int __matherr (struct exception *__exc);

# if !defined NOT_IN_libc || defined IS_IN_libm
hidden_proto (__finite)
hidden_proto (__isinf)
hidden_proto (__isnan)
hidden_proto (__finitef)
hidden_proto (__isinff)
hidden_proto (__isnanf)

#  ifndef __NO_LONG_DOUBLE_MATH
hidden_proto (__finitel)
hidden_proto (__isinfl)
hidden_proto (__isnanl)
#  endif
# endif

libm_hidden_proto (__fpclassify)
libm_hidden_proto (__fpclassifyf)
libm_hidden_proto (__exp)
libm_hidden_proto (__expf)

# ifndef __NO_LONG_DOUBLE_MATH
libm_hidden_proto (__fpclassifyl)
libm_hidden_proto (__expl)
libm_hidden_proto (__expm1l)
# endif

extern int __isinf_ns (double);
extern int __isinf_nsf (float);
extern int __isinf_nsl (long double);

#endif
