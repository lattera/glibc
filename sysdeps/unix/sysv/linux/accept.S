#define	socket	accept
#define	__socket __libc_accept
#define	NARGS	3
#define NEED_CANCELLATION
#include <socket.S>
libc_hidden_def (accept)
