#define	socket	recvmsg
#define	__socket __libc_recvmsg
#define	NARGS	3
#include <pt-socket.S>
weak_alias (__libc_recvmsg, __recvmsg)
