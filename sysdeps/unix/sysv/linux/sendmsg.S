#define	socket	sendmsg
#define	NARGS	3
#include <socket.S>
strong_alias (__sendmsg, __libc_sendmsg)
