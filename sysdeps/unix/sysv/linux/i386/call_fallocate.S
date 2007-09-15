#include <sysdep.h>

#ifdef __NR_fallocate
PSEUDO_ERRVAL (__call_fallocate, fallocate, 6)
 ret_ERRVAL
PSEUDO_END_ERRVAL(__call_fallocate)
#endif
