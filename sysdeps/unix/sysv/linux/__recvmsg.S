#define	socket	__recvmsg
#define	__socket __libc_recvmsg
#define SOCKOP___recvmsg SOCKOP_recvmsg
#define	NARGS	3
#include <socket.S>
