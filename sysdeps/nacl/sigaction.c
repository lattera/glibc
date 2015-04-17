#if IS_IN (libpthread)

/* This placeholder file prevents nptl/sigaction.c from being compiled.
   For NaCl, there is no need for a separate sigaction in libpthread.  */

#else

/* Get the standard stub.  */
#include <signal/sigaction.c>

#endif
