#ifndef	_SYS_POLL_H
# include <io/sys/poll.h>

extern int __poll (struct pollfd *__fds, unsigned long int __nfds,
		   int __timeout);
libc_hidden_proto (__poll)

#endif
