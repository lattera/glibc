#ifdef __ARM_NEON__
/* Under __ARM_NEON__, this file defines memcpy directly.  */
libc_hidden_builtin_def (memcpy)
#else
# define memcpy __memcpy_neon
#endif

#define MEMCPY_NEON
#include "memcpy_impl.S"
