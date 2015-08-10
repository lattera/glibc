#define __openat64 __rename___openat64
#define openat64 __rename_openat64

#include "../openat.c"

#undef __openat64
#undef openat64

strong_alias (__openat, __openat64)
hidden_ver (__openat, __openat64)
weak_alias (openat, openat64)
