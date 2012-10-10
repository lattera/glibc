/* Multiple versions of strncmp
   All versions must be listed in ifunc-impl-list.c.  */
#define USE_AS_STRNCMP
#define STRCMP	strncmp
#include "strcmp.S"
