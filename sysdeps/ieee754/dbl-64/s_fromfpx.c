#define UNSIGNED 0
#define INEXACT 1
#define FUNC __fromfpx
#include <s_fromfp_main.c>
weak_alias (__fromfpx, fromfpx)
#ifdef NO_LONG_DOUBLE
strong_alias (__fromfpx, __fromfpxl)
weak_alias (__fromfpx, fromfpxl)
#endif
