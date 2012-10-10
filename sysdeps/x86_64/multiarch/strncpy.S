/* Multiple versions of strncpy
   All versions must be listed in ifunc-impl-list.c.  */
#define STRCPY strncpy
#define USE_AS_STRNCPY
#include "strcpy.S"
