/* Multiple versions of strpbrk
   All versions must be listed in ifunc-impl-list.c.  */
#define STRCSPN strpbrk
#define USE_AS_STRPBRK
#include "strcspn.S"
