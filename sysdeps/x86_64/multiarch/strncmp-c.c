#ifdef SHARED
#define STRNCMP __strncmp_sse2
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__strncmp_sse2, __GI_strncmp, __strncmp_sse2);
#endif

#include "strncmp.c"
