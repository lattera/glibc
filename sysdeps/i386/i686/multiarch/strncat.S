/* Multiple versions of strncat
   All versions must be listed in ifunc-impl-list.c.  */
#define STRCAT strncat
#define USE_AS_STRNCAT
#include "strcat.S"
