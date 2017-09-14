#define SIG 1
#define FUNC __setpayloadsig
#include <s_setpayload_main.c>
weak_alias (__setpayloadsig, setpayloadsig)
#ifdef NO_LONG_DOUBLE
strong_alias (__setpayloadsig, __setpayloadsigl)
weak_alias (__setpayloadsig, setpayloadsigl)
#endif
