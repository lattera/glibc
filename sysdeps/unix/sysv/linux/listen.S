#define	socket	listen
#define	NARGS	2
#define NO_WEAK_ALIAS	1
#include <socket.S>
weak_alias (listen, __listen)
