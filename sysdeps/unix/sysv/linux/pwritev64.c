#define PWRITEV pwritev64
#define PWRITEV_REPLACEMENT __atomic_pwritev64_replacement
#define PWRITE __pwrite64
#define OFF_T off64_t

#include "pwritev.c"
