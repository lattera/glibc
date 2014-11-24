#if IS_IN (libc)
# define wcscpy  __wcscpy_sse2
#endif

#include "wcsmbs/wcscpy.c"
