#define	socket	recvfrom
#define	__socket __libc_recvfrom
#define	NARGS	6
#define NEED_CANCELLATION
#include <socket.S>
weak_alias (__libc_recvfrom, __recvfrom)
