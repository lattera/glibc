#define	socket	bind
#define	NARGS	3
#define NO_WEAK_ALIAS	1
#include <socket.S>
weak_alias (bind, __bind)
