#define	socket	setsockopt
#define	NARGS	5
#define NO_WEAK_ALIAS	1
#include <socket.S>
weak_alias (setsockopt, __setsockopt)
