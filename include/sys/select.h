#ifndef _SYS_SELECT_H
#include <misc/sys/select.h>

/* Now define the internal interfaces.  */
extern int __pselect (int __nfds, __fd_set *__readfds,
		      __fd_set *__writefds, __fd_set *__exceptfds,
		      const struct timespec *__timeout,
		      const __sigset_t *__sigmask);

extern int __select (int __nfds, __fd_set *__restrict __readfds,
		     __fd_set *__restrict __writefds,
		     __fd_set *__restrict __exceptfds,
		     struct timeval *__restrict __timeout);
#endif
