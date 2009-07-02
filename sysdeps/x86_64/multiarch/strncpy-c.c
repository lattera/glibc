#define STRNCPY __strncpy_sse2
#ifdef SHARED
#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__strncpy_sse2, __GI_strncpy, __strncpy_sse2);
#endif

#include "strncpy.c"
