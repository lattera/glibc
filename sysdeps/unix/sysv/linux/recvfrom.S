#define	socket	recvfrom
#define	NARGS	6
#include <socket.S>
strong_alias (__recvfrom, __libc_recvfrom)
