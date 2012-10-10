/* Multiple versions of strcasestr
   All versions must be listed in ifunc-impl-list.c.  */
#define __strcasestr_sse2 __strcasestr_ia32
#include <sysdeps/x86_64/multiarch/strcasestr-c.c>
