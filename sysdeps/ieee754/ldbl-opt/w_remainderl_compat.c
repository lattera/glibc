#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_remainderl_compat.c>
#if LIBM_SVID_COMPAT
strong_alias (__remainderl, __dreml)
long_double_symbol (libm, __dreml, dreml);
#endif
