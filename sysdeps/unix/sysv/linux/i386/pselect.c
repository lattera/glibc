#include <sys/select.h>

extern int __call_pselect6 (int nfds, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, const struct timespec *timeout,
			    void *data) attribute_hidden;


#define CALL_PSELECT6(nfds, readfds, writefds, exceptfds, timeout, data) \
  ({ int r = __call_pselect6 (nfds, readfds, writefds, exceptfds, timeout,    \
			      data);					      \
     if (r < 0 && r > -4096)						      \
       {								      \
	 __set_errno (-r);						      \
	 r = -1;							      \
       }								      \
     r; })

#include "../pselect.c"
