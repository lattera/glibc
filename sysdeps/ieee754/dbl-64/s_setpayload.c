#define SIG 0
#define FUNC setpayload
#include <s_setpayload_main.c>
#ifdef NO_LONG_DOUBLE
weak_alias (setpayload, setpayloadl)
#endif
