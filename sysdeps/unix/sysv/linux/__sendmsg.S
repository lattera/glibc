#define	socket	__sendmsg
#define	__socket __libc_sendmsg
#define SOCKOP___sendmsg SOCKOP_sendmsg
#define	NARGS	3
#include <socket.S>
