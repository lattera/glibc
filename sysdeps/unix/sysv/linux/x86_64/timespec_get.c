#include "bits/libc-vdso.h"

#ifdef SHARED
# define INTERNAL_GETTIME(id, tp) \
  ({ long int (*f) (clockid_t, struct timespec *) = __vdso_clock_gettime; \
  PTR_DEMANGLE (f);							  \
  f (id, tp); })
#endif

#include "../timespec_get.c"
