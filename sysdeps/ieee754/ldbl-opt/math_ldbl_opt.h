/* -mlong-double-64 compatibility mode macros.  */

#ifndef NLDBL_VERSION
# define NLDBL_VERSION GLIBC_2_4
#endif

#include <math.h>
#include <math/math_private.h>
#include <shlib-compat.h>
#define LONG_DOUBLE_COMPAT(lib, introduced) \
  SHLIB_COMPAT(lib, introduced, NLDBL_VERSION)
#define long_double_symbol(lib, local, symbol) \
  long_double_symbol_1 (lib, local, symbol, NLDBL_VERSION)
#if defined HAVE_ELF && defined SHARED && defined DO_VERSIONING
# define ldbl_hidden_def(local, name) libc_hidden_ver (local, name)
# define ldbl_strong_alias(name, aliasname) \
  strong_alias (name, __GL_##name##_##aliasname) \
  long_double_symbol (libc, __GL_##name##_##aliasname, aliasname);
# define ldbl_weak_alias(name, aliasname) \
  weak_alias (name, __GL_##name##_##aliasname) \
  long_double_symbol (libc, __GL_##name##_##aliasname, aliasname);
# define long_double_symbol_1(lib, local, symbol, version) \
  versioned_symbol (lib, local, symbol, version)
#elif defined HAVE_WEAK_SYMBOLS
# define ldbl_hidden_def(local, name) libc_hidden_def (name)
# define ldbl_strong_alias(name, aliasname) strong_alias (name, aliasname)
# define ldbl_weak_alias(name, aliasname) weak_alias (name, aliasname)
/* Note that weak_alias cannot be used - it is defined to nothing
   in most of the files.  */
# define long_double_symbol_1(lib, local, symbol, version) \
  _weak_alias (local, symbol)
#else
# define ldbl_hidden_def(local, name) libc_hidden_def (name)
# define ldbl_strong_alias(name, aliasname) strong_alias (name, aliasname)
# define ldbl_weak_alias(name, aliasname) strong_alias (name, aliasname)
# define long_double_symbol_1(lib, local, symbol, version) \
  strong_alias (local, symbol)
#endif

/* Set temporarily to non-zero if long double should be considered
   the same as double.  */
extern __thread int __no_long_double attribute_tls_model_ie attribute_hidden;
#define __ldbl_is_dbl __builtin_expect (__no_long_double, 0)
