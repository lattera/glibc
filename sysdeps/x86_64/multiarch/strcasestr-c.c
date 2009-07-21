#include "init-arch.h"

#define STRCASESTR __strcasestr_sse2
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__strcasestr_sse2, __GI_strcasestr, __strcasestr_sse2);

#include "string/strcasestr.c"

extern char *__strcasestr_sse42 (const char *, const char *);

#if 1
libc_ifunc (__strcasestr,
	    HAS_SSE4_2 ? __strcasestr_sse42 : __strcasestr_sse2);
#else
libc_ifunc (__strcasestr,
	    0 ? __strcasestr_sse42 : __strcasestr_sse2);
#endif
