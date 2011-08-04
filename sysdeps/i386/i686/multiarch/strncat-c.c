#define STRNCAT __strncat_ia32
#ifdef SHARED
#undef libc_hidden_def
#define libc_hidden_def(name) \
  __hidden_ver1 (__strncat_ia32, __GI___strncat, __strncat_ia32);
#endif

#include "string/strncat.c"
