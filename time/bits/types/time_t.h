#ifndef __time_t_defined
#define __time_t_defined 1

#include <bits/types.h>

__BEGIN_NAMESPACE_STD
/* Returned by `time'.  */
typedef __time_t time_t;
__END_NAMESPACE_STD
#ifdef __USE_POSIX
__USING_NAMESPACE_STD(time_t)
#endif

#endif
