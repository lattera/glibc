#define memcpy __mempcpy
#include <sysdeps/i386/i586/memcpy.S>

weak_alias (__mempcpy, mempcpy)
