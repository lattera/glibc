#ifndef _SYS_SELECT_H
#include <misc/sys/select.h>

/* Now define the internal interfaces.  */
extern int __pselect __P ((int __nfds, __fd_set *__readfds,
			   __fd_set *__writefds, __fd_set *__exceptfds,
			   const struct timespec *__timeout,
			   const __sigset_t *__sigmask));
#endif
