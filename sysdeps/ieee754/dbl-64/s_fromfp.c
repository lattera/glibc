#define UNSIGNED 0
#define INEXACT 0
#define FUNC fromfp
#include <s_fromfp_main.c>
#ifdef NO_LONG_DOUBLE
weak_alias (fromfp, fromfpl)
#endif
