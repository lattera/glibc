#ifndef _ERRNO_H

#include <stdlib/errno.h>

#ifdef _ERRNO_H

# include <tls.h>		/* Defines USE_TLS.  */

# if USE_TLS && HAVE___THREAD
#  undef  errno
#  define errno errno		/* For #ifndef errno tests.  */
extern __thread int errno;
#  define __set_errno(val) (errno = (val))
# else
#  define __set_errno(val) (*__errno_location ()) = (val)
# endif

#endif /* _ERRNO_H */

#endif /* ! _ERRNO_H */
