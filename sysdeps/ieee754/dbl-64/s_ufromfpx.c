#define UNSIGNED 1
#define INEXACT 1
#define FUNC ufromfpx
#include <s_fromfp_main.c>
#ifdef NO_LONG_DOUBLE
weak_alias (ufromfpx, ufromfpxl)
#endif
