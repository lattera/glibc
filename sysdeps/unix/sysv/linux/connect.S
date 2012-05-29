#define	socket	connect
#define __socket __libc_connect
#define	NARGS	3
#define NEED_CANCELLATION
#include <socket.S>
weak_alias (__libc_connect, __connect)
libc_hidden_weak (__connect)
