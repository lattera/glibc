/* This produces a makefile fragment saying `use-thread = yes' or no.  */

#include <tls.h>

#if USE_TLS
@@@ use-tls = yes @@@
#else
@@@ use-tls = no @@@
#endif

#if USE___THREAD
@@@ use-thread = yes @@@
#else
@@@ use-thread = no @@@
#endif
