#include <sysdeps/unix/sysv/signal.S>

/* Used in siginterrupt() */

	.data
	.globl _sigintr
_sigintr:
	.long 0
