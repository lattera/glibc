#define	socket	connect
#define __socket __libc_connect
#define	NARGS	3
#include <socket.S>
weak_alias (__libc_connect, __connect)
