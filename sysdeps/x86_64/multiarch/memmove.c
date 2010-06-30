#ifndef NOT_IN_libc
#include "init-arch.h"

#define MEMMOVE __memmove_sse2
#ifdef SHARED
# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__memmove_sse2, __GI_memmove, __memmove_sse2);
#endif
#endif

#include "string/memmove.c"

#ifndef NOT_IN_libc
extern __typeof (__memmove_sse2) __memmove_sse2 attribute_hidden;
extern __typeof (__memmove_sse2) __memmove_ssse3 attribute_hidden;
extern __typeof (__memmove_sse2) __memmove_ssse3_back attribute_hidden;

libc_ifunc (memmove,
	    HAS_SSSE3
	    ? (HAS_FAST_COPY_BACKWARD
	       ? __memmove_ssse3_back : __memmove_ssse3)
	    : __memmove_sse2);
#endif
