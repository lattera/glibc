#define	socket	sendmsg
#define	__socket __libc_sendmsg
#define	NARGS	3
#include <socket.S>
