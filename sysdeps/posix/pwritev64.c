#include <bits/wordsize.h>

#if __WORDSIZE == 32
# define PWRITEV pwritev64
# define PWRITE __pwrite64
# define OFF_T off64_t

# include "pwritev.c"
#endif
