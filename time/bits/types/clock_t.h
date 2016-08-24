#ifndef __clock_t_defined
#define __clock_t_defined 1

#include <bits/types.h>

__BEGIN_NAMESPACE_STD
/* Returned by `clock'.  */
typedef __clock_t clock_t;
__END_NAMESPACE_STD

#if defined __USE_XOPEN || defined __USE_POSIX
__USING_NAMESPACE_STD(clock_t)
#endif

#endif
