#include <stdlib/errno.h>

#ifdef _ERRNO_H

#if USE_TLS && HAVE___THREAD
# undef errno
extern __thread int errno;
# define __set_errno(val) (errno = (val))
#else
# define __set_errno(val) (*__errno_location ()) = (val)
#endif

#endif
