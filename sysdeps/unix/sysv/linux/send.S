#define	socket	send
#define	__socket __libc_send
#define	NARGS	4
#define NEED_CANCELLATION
#include <socket.S>
weak_alias (__libc_send, __send)
libc_hidden_def (__send)
