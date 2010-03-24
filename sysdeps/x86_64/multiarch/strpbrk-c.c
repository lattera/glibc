/* Don't define multiple versions for strpbrk in static library since we
   need strpbrk before the initialization happened.  */
#ifdef SHARED
# define USE_AS_STRPBRK
# define STRCSPN_SSE2 __strpbrk_sse2
# define STRCSPN_SSE42 __strpbrk_sse42
# include "strcspn-c.c"
#endif
