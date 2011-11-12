#include <wchar.h>

#ifndef NOT_IN_libc
# define WMEMCMP  __wmemcmp_ia32
#endif

extern __typeof (wmemcmp) __wmemcmp_ia32;

#include "wcsmbs/wmemcmp.c"
