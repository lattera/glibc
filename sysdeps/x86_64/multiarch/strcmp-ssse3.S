#ifndef NOT_IN_libc
# define USE_SSSE3 1
# define STRCMP __strcmp_ssse3
# include "../strcmp.S"
#endif
