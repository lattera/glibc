#include <tst-tls1.h>

#ifdef TLS_REGISTER
/* Ensure tls_registry is exported from the binary.  */
void *tst_tls1mod attribute_hidden = tls_registry;
#endif
