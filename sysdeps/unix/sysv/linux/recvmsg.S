#define	socket	recvmsg
#define	NARGS	3
#include <socket.S>
strong_alias (__recvmsg, __libc_recvmsg)
