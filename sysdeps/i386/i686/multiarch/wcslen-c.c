#include <wchar.h>

#ifndef NOT_IN_libc
# define WCSLEN  __wcslen_ia32
#endif

extern __typeof (wcslen) __wcslen_ia32;

#include "wcsmbs/wcslen.c"
