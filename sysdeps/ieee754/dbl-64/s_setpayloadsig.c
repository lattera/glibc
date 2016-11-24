#define SIG 1
#define FUNC setpayloadsig
#include <s_setpayload_main.c>
#ifdef NO_LONG_DOUBLE
weak_alias (setpayloadsig, setpayloadsigl)
#endif
