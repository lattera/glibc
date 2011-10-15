#ifndef _CTYPE_H

/* Initialize ctype locale data.  */
extern void __ctype_init (void);
libc_hidden_proto (__ctype_init)

extern int __isctype (int __c, int __mask);

# ifndef NOT_IN_libc

/* These accessors are used by the optimized macros to find the
   thread-local cache of ctype information from the current thread's
   locale.  For inside libc, define them as inlines using the _NL_CURRENT
   accessors.  We don't use _NL_CURRENT_LOCALE->__ctype_b here because we
   want to cause a link-time ref to _nl_current_LC_CTYPE under
   NL_CURRENT_INDIRECT.  */

#  include "../locale/localeinfo.h"
#  include <bits/libc-tsd.h>

#  ifndef CTYPE_EXTERN_INLINE	/* Used by ctype/ctype-info.c, which see.  */
#   define CTYPE_EXTERN_INLINE extern inline
#  endif

__libc_tsd_define (extern, const uint16_t *, CTYPE_B)
__libc_tsd_define (extern, const int32_t *, CTYPE_TOUPPER)
__libc_tsd_define (extern, const int32_t *, CTYPE_TOLOWER)


CTYPE_EXTERN_INLINE const uint16_t ** __attribute__ ((const))
__ctype_b_loc (void)
{
  return __libc_tsd_address (const uint16_t *, CTYPE_B);
}

CTYPE_EXTERN_INLINE const int32_t ** __attribute__ ((const))
__ctype_toupper_loc (void)
{
  return __libc_tsd_address (const int32_t *, CTYPE_TOUPPER);
}

CTYPE_EXTERN_INLINE const int32_t ** __attribute__ ((const))
__ctype_tolower_loc (void)
{
  return __libc_tsd_address (const int32_t *, CTYPE_TOLOWER);
}

# endif	/* Not NOT_IN_libc.  */

# include <ctype/ctype.h>

# if !defined __NO_CTYPE && !defined NOT_IN_libc
/* The spec says that isdigit must only match the decimal digits.  We
   can check this without a memory access.  */
#  undef isdigit
#  define isdigit(c) ({ int __c = (c); __c >= '0' && __c <= '9'; })
#  undef isdigit_l
#  define isdigit_l(c, l) ({ int __c = (c); __c >= '0' && __c <= '9'; })
#  undef __isdigit_l
#  define __isdigit_l(c, l) ({ int __c = (c); __c >= '0' && __c <= '9'; })
# endif

#endif /* ctype.h */
