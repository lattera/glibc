#define	socket	sendto
#define	__socket __libc_sendto
#define	NARGS	6
#define NEED_CANCELLATION
#include <socket.S>
weak_alias (__libc_sendto, __sendto)
