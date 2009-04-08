#define PREADV preadv64
#define PREADV_REPLACEMENT __atomic_preadv64_replacement
#define PREAD __libc_pread64
#define OFF_T off64_t

#include "preadv.c"
