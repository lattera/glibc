#define memset __bzero
#include <sysdeps/i386/i686/memset.S>
weak_alias (BP_SYM (__bzero), BP_SYM (bzero))
