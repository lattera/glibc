#define __openat64 __rename___openat64
#define __openat64_2 __rename___openat64_2
#define __openat64_nocancel __rename___openat64_nocancel
#define openat64 __rename_openat64

#include "../openat.c"

#undef __openat64
#undef __openat64_2
#undef __openat64_nocancel
#undef openat64

weak_alias (__openat, __openat64)
weak_alias (__openat_2, __openat64_2)
weak_alias (__openat_nocancel, __openat64_nocancel)
weak_alias (openat, openat64)
