#define UNSIGNED 1
#define INEXACT 1
#define FUNC __ufromfpxf128
#include <float128_private.h>
#include "../ldbl-128/s_fromfpl_main.c"
weak_alias (__ufromfpxf128, ufromfpxf128)
