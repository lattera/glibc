#define	socket	accept
#define	NARGS	3
#include <socket.S>
strong_alias (__accept, __libc_accept)
