#define	socket	sendto
#define	__socket __libc_sendto
#define	NARGS	6
#include <socket.S>
