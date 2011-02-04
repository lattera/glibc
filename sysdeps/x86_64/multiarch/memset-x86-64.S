#include <sysdep.h>

#ifndef NOT_IN_libc
# undef ENTRY_CHK
# define ENTRY_CHK(name) \
	.type __memset_chk_x86_64, @function; \
	.globl __memset_chk_x86_64; \
	.p2align 4; \
	__memset_chk_x86_64: cfi_startproc; \
	CALL_MCOUNT
# undef END_CHK
# define END_CHK(name) \
	cfi_endproc; .size __memset_chk_x86_64, .-__memset_chk_x86_64

# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)
# define memset __memset_x86_64
# include "../memset.S"
#endif
