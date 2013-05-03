#define __openat64 __rename___openat64
#define __openat64_nocancel __rename___openat64_nocancel
#define openat64 __rename_openat64

#include "../openat.c"

#undef __openat64
#undef __openat64_nocancel
#undef openat64

strong_alias (__openat, __openat64)
hidden_ver (__openat, __openat64)
strong_alias (__openat_nocancel, __openat64_nocancel)
weak_alias (openat, openat64)
