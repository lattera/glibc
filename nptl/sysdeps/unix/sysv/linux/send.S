#define	socket	send
#define	__socket __libc_send
#define	NARGS	4
#include <pt-socket.S>
weak_alias (__libc_send, __send)
libc_hidden_def (__send)
