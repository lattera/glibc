#define	socket	sendmsg
#define	__socket __libc_sendmsg
#define	NARGS	3
#include <pt-socket.S>
weak_alias (__libc_sendmsg, __sendmsg)
