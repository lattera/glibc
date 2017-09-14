#define UNSIGNED 0
#define INEXACT 0
#define FUNC __fromfpf128
#include <float128_private.h>
#include "../ldbl-128/s_fromfpl_main.c"
weak_alias (__fromfpf128, fromfpf128)
