#include "bits/libc-vdso.h"

#ifdef SHARED
# define SYSCALL_GETTIME(id, tp) \
  (*__vdso_clock_gettime) (id, tp)
# define INTERNAL_GETTIME(id, tp) \
  (*__vdso_clock_gettime) (id, tp)
#endif

#include "../clock_gettime.c"
