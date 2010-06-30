#include <sysdep.h>

	.text
ENTRY(bcopy)
	xchg	%rdi, %rsi
	jmp	HIDDEN_BUILTIN_JUMPTARGET(memmove)
END(bcopy)
