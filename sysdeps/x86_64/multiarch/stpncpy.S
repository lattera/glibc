/* Multiple versions of stpncpy
   All versions must be listed in ifunc-impl-list.c.  */
#define STRCPY __stpncpy
#define USE_AS_STPCPY
#define USE_AS_STRNCPY
#include "strcpy.S"

weak_alias (__stpncpy, stpncpy)
