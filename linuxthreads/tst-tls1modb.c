#include <tst-tls1.h>

#ifdef TLS_REGISTER
static __thread int b;
TLS_REGISTER (b)
#endif
