#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#undef compat_symbol
#define compat_symbol(l,n,a,v)
#include <math/w_exp10l_compat.c>
#if LIBM_SVID_COMPAT
# if !LONG_DOUBLE_COMPAT (libm, GLIBC_2_1)
/* If ldbl-opt is used without special versioning for exp10l being
   required, the generic code does not define exp10l because of the
   undefine and redefine of weak_alias above.  */
#  undef weak_alias
#  define weak_alias(name, aliasname) _weak_alias (name, aliasname)
weak_alias (__exp10l, exp10l)
# endif
# if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_27)
/* compat_symbol was undefined and redefined above to avoid the
   default pow10l compat symbol at version GLIBC_2_1 (as for ldbl-opt
   configurations, that version should have the alias to exp10).  So
   it now needs to be redefined to define the compat symbol at version
   LONG_DOUBLE_COMPAT_VERSION.  */
#  undef compat_symbol
#  define compat_symbol(lib, local, symbol, version)	\
  compat_symbol_reference (lib, local, symbol, version)
compat_symbol (libm, __pow10l, pow10l, LONG_DOUBLE_COMPAT_VERSION);
# endif
#endif
