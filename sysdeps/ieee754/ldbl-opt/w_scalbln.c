#include <math-type-macros-double.h>
#include <w_scalbln_template.c>
#if IS_IN (libc) && LONG_DOUBLE_COMPAT (libc, GLIBC_2_1)
compat_symbol (libc, __w_scalbln, scalblnl, GLIBC_2_1);
#endif
