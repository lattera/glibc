#define	socket	send
#define	__socket __libc_send
#define	NARGS	4
#include <socket.S>
weak_alias (__libc_send, __send)
