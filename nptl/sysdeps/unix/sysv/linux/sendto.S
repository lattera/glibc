#define	socket	sendto
#define	__socket __libc_sendto
#define	NARGS	6
#include <pt-socket.S>
weak_alias (__libc_sendto, __sendto)
