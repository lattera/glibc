#define UNSIGNED 1
#define INEXACT 0
#define FUNC ufromfp
#include <s_fromfp_main.c>
#ifdef NO_LONG_DOUBLE
weak_alias (ufromfp, ufromfpl)
#endif
