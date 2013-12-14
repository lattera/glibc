/* Multiple versions of strcasestr
   All versions must be listed in ifunc-impl-list.c.  */

#include "init-arch.h"

#define STRCASESTR __strcasestr_sse2

#include "string/strcasestr.c"

extern __typeof (__strcasestr_sse2) __strcasestr_sse2 attribute_hidden;

libc_ifunc (__strcasestr,
	    __strcasestr_sse2);
