#include <tst-tls1.h>

#ifdef TLS_REGISTER
static __thread int d;
TLS_REGISTER (d)
#endif
