#define UNSIGNED 1
#define INEXACT 0
#define FUNC __ufromfp
#include <s_fromfp_main.c>
weak_alias (__ufromfp, ufromfp)
#ifdef NO_LONG_DOUBLE
strong_alias (__ufromfp, __ufromfpl)
weak_alias (__ufromfp, ufromfpl)
#endif
