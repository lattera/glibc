#define memcpy __mempcpy
#include <sysdeps/x86_64/memcpy.S>

libc_hidden_def (BP_SYM (__mempcpy))
weak_alias (BP_SYM (__mempcpy), BP_SYM (mempcpy))
libc_hidden_builtin_def (mempcpy)
