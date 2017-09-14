#include <float128_private.h>
#define SIG 1
#define FUNC __setpayloadsigf128
#include "../ldbl-128/s_setpayloadl_main.c"
weak_alias (__setpayloadsigf128, setpayloadsigf128)
