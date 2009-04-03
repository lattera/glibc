#include <bits/wordsize.h>

#if __WORDSIZE == 32
# define PREADV preadv64
# define PREAD __pread64
# define OFF_T off64_t

# include "preadv.c"
#endif
