#define	socket	sendto
#define	NARGS	6
#include <socket.S>
strong_alias (__sendto, __libc_sendto)
