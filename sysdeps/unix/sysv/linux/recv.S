#define	socket	recv
#define	__socket __libc_recv
#define	NARGS	4
#define NEED_CANCELLATION
#include <socket.S>
weak_alias (__libc_recv, __recv)
