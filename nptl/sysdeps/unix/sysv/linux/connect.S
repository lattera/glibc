#define	socket	connect
#define __socket __libc_connect
#define	NARGS	3
#include <pt-socket.S>
strong_alias (__libc_connect, __connect_internal)
weak_alias (__libc_connect, __connect)
