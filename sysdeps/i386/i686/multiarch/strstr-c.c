#include "init-arch.h"

#define STRSTR __strstr_ia32
#if defined SHARED && defined DO_VERSIONING && !defined NO_HIDDEN
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__strstr_ia32, __GI_strstr, __strstr_ia32);
#endif

#include "string/strstr.c"

extern char *__strstr_sse42 (const char *, const char *) attribute_hidden;
extern __typeof (__strstr_ia32) __strstr_ia32 attribute_hidden;

libc_ifunc (strstr, HAS_SSE4_2 ? __strstr_sse42 : __strstr_ia32);
