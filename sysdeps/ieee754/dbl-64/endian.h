#include <endian.h>

#if __FLOAT_WORD_ORDER == __BIG_ENDIAN
#define HIGH_HALF 0
#define  LOW_HALF 1
#else
#if __FLOAT_WORD_ORDER == __LITTLE_ENDIAN
#define HIGH_HALF 1
#define  LOW_HALF 0
#endif
#endif
