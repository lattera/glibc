#ifndef _ERRNO_H

#include <stdlib/errno.h>

#if defined _ERRNO_H && !defined _ISOMAC

# ifdef IS_IN_rtld
#  include <dl-sysdep.h>
# endif

# if RTLD_PRIVATE_ERRNO
/* The dynamic linker uses its own private errno variable.
   All access to errno inside the dynamic linker is serialized,
   so a single (hidden) global variable is all it needs.  */

#  undef  errno
#  define errno errno		/* For #ifndef errno tests.  */
extern int errno attribute_hidden;
#  define __set_errno(val) (errno = (val))

# else

#  include <tls.h>		/* Defines USE_TLS.  */

#  if USE___THREAD
#   undef  errno
#   define errno errno		/* For #ifndef errno tests.  */
extern __thread int errno;
#   define __set_errno(val) (errno = (val))
#  else
#   define __set_errno(val) (*__errno_location ()) = (val)
#  endif

# endif	/* RTLD_PRIVATE_ERRNO */

#endif /* _ERRNO_H */

#endif /* ! _ERRNO_H */
