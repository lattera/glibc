#ifndef _CTYPE_H

# include <ctype/ctype.h>

extern int __isctype (int __c, int __mask);

# ifndef NOT_IN_libc

/* The optimized macros are not defined for users because they can't see
   the thread-local locale state.  For inside libc, define them using the
   _NL_CURRENT accessors.  We don't use _NL_CURRENT_LOCALE->__ctype_b here
   because we want to cause a link-time ref to _nl_current_LC_CTYPE under
   NL_CURRENT_INDIRECT.  */

#  include "../locale/localeinfo.h"
#  define __isctype(c, type) \
     (((uint16_t *) _NL_CURRENT (LC_CTYPE, _NL_CTYPE_CLASS) + 128) \
      [(int) (c)] & (uint16_t) type)
#  ifndef __NO_CTYPE
#   define isalnum(c)	__isctype((c), _ISalnum)
#   define isalpha(c)	__isctype((c), _ISalpha)
#   define iscntrl(c)	__isctype((c), _IScntrl)
#   define isdigit(c)	__isctype((c), _ISdigit)
#   define islower(c)	__isctype((c), _ISlower)
#   define isgraph(c)	__isctype((c), _ISgraph)
#   define isprint(c)	__isctype((c), _ISprint)
#   define ispunct(c)	__isctype((c), _ISpunct)
#   define isspace(c)	__isctype((c), _ISspace)
#   define isupper(c)	__isctype((c), _ISupper)
#   define isxdigit(c)	__isctype((c), _ISxdigit)
#   define isblank(c)	__isctype((c), _ISblank)

#   define tolower(c) \
      __tobody (c, tolower, \
		(uint32_t *) _NL_CURRENT (LC_CTYPE, _NL_CTYPE_TOLOWER) + 128, \
		(c))
#   define _tolower(c) tolower (c)
#   define toupper(c) \
      __tobody (c, toupper, \
		(uint32_t *) _NL_CURRENT (LC_CTYPE, _NL_CTYPE_TOUPPER) + 128, \
		(c))
#   define _toupper(c) toupper (c)

#  endif /* Not __NO_CTYPE.  */
# endif	/* _LIBC_REENTRANT.  */

#endif /* ctype.h */
