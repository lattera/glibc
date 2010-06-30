#include "init-arch.h"

#define MEMMOVE_CHK __memmove_chk_sse2

#include "debug/memmove_chk.c"

extern __typeof (__memmove_chk_sse2) __memmove_chk_sse2 attribute_hidden;
extern __typeof (__memmove_chk_sse2) __memmove_chk_ssse3 attribute_hidden;
extern __typeof (__memmove_chk_sse2) __memmove_chk_ssse3_back attribute_hidden;

libc_ifunc (__memmove_chk,
	    HAS_SSSE3
	    ? (HAS_FAST_COPY_BACKWARD
	       ? __memmove_chk_ssse3_back : __memmove_chk_ssse3)
	    : __memmove_chk_sse2);
