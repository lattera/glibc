#include <sysdep.h>

	.globl __socket
PSEUDO(__socket, socket, 3)

PSEUDO_END(__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
