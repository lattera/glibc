#define	socket	accept
#define	__socket __libc_accept
#define	NARGS	3
#include <socket.S>
weak_alias (__libc_accept, __accept)
