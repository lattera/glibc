#ifdef SHARED
# define USE_SSSE3 1
# define STRCMP __strncmp_ssse3
# define USE_AS_STRNCMP
# include "../strcmp.S"
#endif
