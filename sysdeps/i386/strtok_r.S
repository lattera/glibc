#define FUNCTION __strtok_r
#define USE_AS_STRTOK_R	1
#include <sysdeps/i386/strtok.S>
weak_alias (__strtok_r, strtok_r)
strong_alias (__strtok_r, __GI___strtok_r)
