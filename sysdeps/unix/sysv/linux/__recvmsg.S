#define	socket	___syscall_recvmsg
#define	__socket __syscall_recvmsg
#define SOCKOP____syscall_recvmsg SOCKOP_recvmsg
#define	NARGS	3
#include <socket.S>
