#ifndef _CTYPE_H

extern int __isctype (int __c, int __mask);

# include <ctype/ctype.h>

# ifndef NOT_IN_libc

/* The optimized macros are not defined for users because they can't see
   the thread-local locale state.  For inside libc, define them using the
   _NL_CURRENT accessors.  We don't use _NL_CURRENT_LOCALE->__ctype_b here
   because we want to cause a link-time ref to _nl_current_LC_CTYPE under
   NL_CURRENT_INDIRECT.  */

#  include "../locale/localeinfo.h"
#  ifndef __NO_CTYPE
#   undef __isctype
#   define __isctype(c, type) \
     (((uint16_t *) _NL_CURRENT (LC_CTYPE, _NL_CTYPE_CLASS) + 128) \
      [(int) (c)] & (uint16_t) type)

#   undef tolower
#   define tolower(c) \
      __tobody (c, tolower, \
		(uint32_t *) _NL_CURRENT (LC_CTYPE, _NL_CTYPE_TOLOWER) + 128, \
		(c))
#   undef _tolower
#   define _tolower(c) tolower (c)
#   undef toupper
#   define toupper(c) \
      __tobody (c, toupper, \
		(uint32_t *) _NL_CURRENT (LC_CTYPE, _NL_CTYPE_TOUPPER) + 128, \
		(c))
#   undef _toupper
#   define _toupper(c) toupper (c)

#  endif /* Not __NO_CTYPE.  */
# endif	/* _LIBC_REENTRANT.  */

#endif /* ctype.h */
