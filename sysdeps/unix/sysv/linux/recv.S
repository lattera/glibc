#define	socket	recv
#define	NARGS	4
#include <socket.S>
strong_alias (__recv, __libc_recv)
