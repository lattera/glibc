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
#  define errno rtld_errno
extern int rtld_errno attribute_hidden;

# else

#  include <tls.h>		/* Defines USE_TLS.  */

#  if USE___THREAD
#   undef  errno
#   ifndef NOT_IN_libc
#    define errno __libc_errno
#   else
#    define errno errno		/* For #ifndef errno tests.  */
#   endif
extern __thread int errno attribute_tls_model_ie;
#  endif

# endif	/* RTLD_PRIVATE_ERRNO */

# define __set_errno(val) (errno = (val))

# ifndef __ASSEMBLER__
extern int *__errno_location (void) __THROW __attribute__ ((__const__))
#  if RTLD_PRIVATE_ERRNO
     attribute_hidden
#  endif
;
libc_hidden_proto (__errno_location)
# endif

#endif /* _ERRNO_H */

#endif /* ! _ERRNO_H */
