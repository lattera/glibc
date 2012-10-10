/* Multiple versions of strncpy
   All versions must be listed in ifunc-impl-list.c.  */
#define USE_AS_STRNCPY
#define STRCPY strncpy
#include "strcpy.S"
