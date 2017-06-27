#ifdef __ARM_NEON__
/* Under __ARM_NEON__, this file defines memchr directly.  */
libc_hidden_builtin_def (memchr)
#else
# define memchr __memchr_neon
#endif

#define MEMCHR_NEON
#include "memchr_impl.S"
