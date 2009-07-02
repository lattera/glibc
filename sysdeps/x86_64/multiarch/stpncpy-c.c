#define STPNCPY __stpncpy_sse2
#ifdef SHARED
#undef libc_hidden_def
#define libc_hidden_def(name) \
  __hidden_ver1 (__stpncpy_sse2, __GI___stpncpy, __stpncpy_sse2);
#endif

#include "stpncpy.c"
