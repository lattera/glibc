#define memcpy __mempcpy
#include <sysdeps/i386/i586/memcpy.S>

weak_alias (BP_SYM (__mempcpy), BP_SYM (mempcpy))
