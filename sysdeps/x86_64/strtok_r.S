#define FUNCTION __strtok_r
#define USE_AS_STRTOK_R	1
#include <sysdeps/x86_64/strtok.S>
weak_alias (BP_SYM (__strtok_r), BP_SYM (strtok_r))
strong_alias (BP_SYM (__strtok_r), BP_SYM (__GI___strtok_r))
