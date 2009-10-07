#include "init-arch.h"

#define STRCASESTR __strcasestr_sse2

#include "string/strcasestr.c"

extern char *__strcasestr_sse42 (const char *, const char *) attribute_hidden;
extern __typeof (__strcasestr_sse2) __strcasestr_sse2 attribute_hidden;

#if 1
libc_ifunc (__strcasestr,
	    HAS_SSE4_2 ? __strcasestr_sse42 : __strcasestr_sse2);
#else
libc_ifunc (__strcasestr,
	    0 ? __strcasestr_sse42 : __strcasestr_sse2);
#endif
