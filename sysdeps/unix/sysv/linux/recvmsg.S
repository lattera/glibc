#define	socket	recvmsg
#define	__socket __libc_recvmsg
#define	NARGS	3
#define NEED_CANCELLATION
#include <socket.S>
weak_alias (__libc_recvmsg, __recvmsg)
