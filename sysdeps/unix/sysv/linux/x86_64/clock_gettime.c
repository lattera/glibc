#include "bits/libc-vdso.h"

#ifdef SHARED
# define SYSCALL_GETTIME(id, tp) \
  ({ long int (*f) (clockid_t, struct timespec *) = __vdso_clock_gettime; \
  long int v_ret;							  \
  PTR_DEMANGLE (f);							  \
  v_ret = f (id, tp);							  \
  if (INTERNAL_SYSCALL_ERROR_P (v_ret, )) {				  \
    __set_errno (INTERNAL_SYSCALL_ERRNO (v_ret, ));			  \
    v_ret = -1;								  \
  }									  \
  v_ret; })
# define INTERNAL_GETTIME(id, tp) \
  ({ long int (*f) (clockid_t, struct timespec *) = __vdso_clock_gettime; \
  PTR_DEMANGLE (f);							  \
  f (id, tp); })
#endif

#include "../clock_gettime.c"
