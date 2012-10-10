/* Multiple versions of strncmp
   All versions must be listed in ifunc-impl-list.c.  */
#define STRCMP strncmp
#define USE_AS_STRNCMP
#include "strcmp.S"
