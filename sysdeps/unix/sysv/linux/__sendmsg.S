#define	socket	___syscall_sendmsg
#define	__socket __syscall_sendmsg
#define SOCKOP____syscall_sendmsg SOCKOP_sendmsg
#define	NARGS	3
#include <socket.S>
