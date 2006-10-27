/* This produces a makefile fragment saying `use-thread = yes' or no.  */

#include <tls.h>

@@@ use-tls = yes @@@

#if USE___THREAD
@@@ use-thread = yes @@@
#else
@@@ use-thread = no @@@
#endif
