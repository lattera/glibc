#include "bits/libc-vdso.h"

#ifdef SHARED
# define SYSCALL_GETTIME(id, tp) \
  ({ long int (*f) (clockid_t, struct timespec *) = __vdso_clock_gettime; \
  PTR_DEMANGLE (f);							  \
  f (id, tp); })
# define INTERNAL_GETTIME(id, tp) \
  ({ long int (*f) (clockid_t, struct timespec *) = __vdso_clock_gettime; \
  PTR_DEMANGLE (f);							  \
  f (id, tp); })
#endif

#include "../clock_gettime.c"
