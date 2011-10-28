#define WCSCMP __wcscmp_ia32
#ifdef SHARED
# undef libc_hidden_def
# define libc_hidden_def(name) \
  __hidden_ver1 (__wcscmp_ia32, __GI_wcscmp, __wcscmp_ia32);
#endif

#include "wcsmbs/wcscmp.c"
