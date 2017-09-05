#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_fmodl_compat.c>
#if LIBM_SVID_COMPAT
long_double_symbol (libm, __fmodl, fmodl);
#endif
