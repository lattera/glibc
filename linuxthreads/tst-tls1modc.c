#include <tst-tls1.h>

#ifdef TLS_REGISTER
static __thread int c;
TLS_REGISTER (c)
#endif
