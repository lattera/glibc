#define	socket	getsockname
#define	NARGS	3
#define NO_WEAK_ALIAS	1
#include <socket.S>
weak_alias (getsockname, __getsockname)
