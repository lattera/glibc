#define SIG 0
#define FUNC __setpayload
#include <s_setpayload_main.c>
weak_alias (__setpayload, setpayload)
#ifdef NO_LONG_DOUBLE
strong_alias (__setpayload, __setpayloadl)
weak_alias (__setpayload, setpayloadl)
#endif
