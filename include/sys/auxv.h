#include <misc/sys/auxv.h>

#ifndef _ISOMAC

extern __typeof (getauxval) __getauxval;
libc_hidden_proto (__getauxval)

#endif  /* !_ISOMAC */
