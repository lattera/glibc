/* -mlong-double-64 compatibility mode macros.  */

#include <nldbl-abi.h>
#ifndef LONG_DOUBLE_COMPAT_VERSION
# error "nldbl-abi.h must define LONG_DOUBLE_COMPAT_VERSION"
#endif

#include <shlib-compat.h>
#define LONG_DOUBLE_COMPAT(lib, introduced) \
  SHLIB_COMPAT(lib, introduced, LONG_DOUBLE_COMPAT_VERSION)
#define long_double_symbol(lib, local, symbol) \
  long_double_symbol_1 (lib, local, symbol, LONG_DOUBLE_COMPAT_VERSION)
#if defined SHARED && defined DO_VERSIONING
# define ldbl_hidden_def(local, name) libc_hidden_ver (local, name)
# define ldbl_strong_alias(name, aliasname) \
  strong_alias (name, __GL_##name##_##aliasname) \
  long_double_symbol (libc, __GL_##name##_##aliasname, aliasname);
# define ldbl_weak_alias(name, aliasname) \
  weak_alias (name, __GL_##name##_##aliasname) \
  long_double_symbol (libc, __GL_##name##_##aliasname, aliasname);
# define long_double_symbol_1(lib, local, symbol, version) \
  versioned_symbol (lib, local, symbol, version)
#else
# define ldbl_hidden_def(local, name) libc_hidden_def (name)
# define ldbl_strong_alias(name, aliasname) strong_alias (name, aliasname)
# define ldbl_weak_alias(name, aliasname) weak_alias (name, aliasname)
# ifndef __ASSEMBLER__
/* Note that weak_alias cannot be used - it is defined to nothing
   in most of the C files.  */
#  define long_double_symbol_1(lib, local, symbol, version) \
  _weak_alias (local, symbol)
# else
#  define long_double_symbol_1(lib, local, symbol, version) \
  weak_alias (local, symbol)
# endif
#endif

#ifndef __ASSEMBLER__
# include <math.h>
# include <math/math_private.h>

/* Set temporarily to non-zero if long double should be considered
   the same as double.  */
extern __thread int __no_long_double attribute_tls_model_ie attribute_hidden;
# define __ldbl_is_dbl __builtin_expect (__no_long_double, 0)
#endif
