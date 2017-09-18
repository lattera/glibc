#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_remainderl_compat.c>
#if LIBM_SVID_COMPAT
# if !LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
/* If ldbl-opt is used without special versioning for remainderl being
   required, the generic code does not define remainderl because of
   the undefine and redefine of weak_alias above.  */
#  undef weak_alias
#  define weak_alias(name, aliasname) _weak_alias (name, aliasname)
weak_alias (__remainderl, remainderl)
# endif
strong_alias (__remainderl, __dreml)
long_double_symbol (libm, __dreml, dreml);
#endif
