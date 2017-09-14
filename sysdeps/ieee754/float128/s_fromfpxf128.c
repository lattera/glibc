#define UNSIGNED 0
#define INEXACT 1
#define FUNC __fromfpxf128
#include <float128_private.h>
#include "../ldbl-128/s_fromfpl_main.c"
weak_alias (__fromfpxf128, fromfpxf128)
