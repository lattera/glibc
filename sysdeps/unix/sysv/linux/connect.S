#define	socket	connect
#define	NARGS	3
#include <socket.S>
strong_alias (__connect, __libc_connect)
