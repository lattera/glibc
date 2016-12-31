#define UNSIGNED 0
#define INEXACT 1
#define FUNC fromfpx
#include <s_fromfp_main.c>
#ifdef NO_LONG_DOUBLE
weak_alias (fromfpx, fromfpxl)
#endif
