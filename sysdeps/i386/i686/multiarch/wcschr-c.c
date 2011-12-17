#ifndef NOT_IN_libc
# undef libc_hidden_def
# define libc_hidden_def(name) \
  __hidden_ver1 (__wcschr_ia32, __GI_wcschr, __wcschr_ia32);
# define WCSCHR  __wcschr_ia32
#endif

#include "wcsmbs/wcschr.c"
