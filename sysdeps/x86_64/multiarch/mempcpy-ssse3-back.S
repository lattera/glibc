#define USE_AS_MEMPCPY
#define MEMCPY		__mempcpy_ssse3_back
#define MEMCPY_CHK	__mempcpy_chk_ssse3_back
#include "memcpy-ssse3-back.S"
