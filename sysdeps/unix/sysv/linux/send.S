#define	socket	send
#define	NARGS	4
#include <socket.S>
strong_alias (__send, __libc_send)
