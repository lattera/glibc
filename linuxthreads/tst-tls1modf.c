#include <tst-tls1.h>

#ifdef TLS_REGISTER
char tst_tls1modf[60] attribute_hidden = { 26 };
static __thread int f1 = 24;
static __thread char f2 [32] __attribute__ ((aligned (64)));
TLS_REGISTER (f1)
TLS_REGISTER (f2)
#endif
