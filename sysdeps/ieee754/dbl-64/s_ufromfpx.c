#define UNSIGNED 1
#define INEXACT 1
#define FUNC __ufromfpx
#include <s_fromfp_main.c>
weak_alias (__ufromfpx, ufromfpx)
#ifdef NO_LONG_DOUBLE
strong_alias (__ufromfpx, __ufromfpxl)
weak_alias (__ufromfpx, ufromfpxl)
#endif
