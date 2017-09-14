#define UNSIGNED 0
#define INEXACT 0
#define FUNC __fromfp
#include <s_fromfp_main.c>
weak_alias (__fromfp, fromfp)
#ifdef NO_LONG_DOUBLE
strong_alias (__fromfp, __fromfpl)
weak_alias (__fromfp, fromfpl)
#endif
