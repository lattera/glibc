#define STRNCAT __strncat_sse2
#ifdef SHARED
#undef libc_hidden_def
#define libc_hidden_def(name) \
  __hidden_ver1 (__strncat_sse2, __GI___strncat, __strncat_sse2);
#endif

#include "string/strncat.c"
