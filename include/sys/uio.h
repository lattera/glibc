#ifndef _SYS_UIO_H
#include <misc/sys/uio.h>

/* Now define the internal interfaces.  */
extern ssize_t __readv (int __fd, __const struct iovec *__vector,
			int __count);
extern ssize_t __libc_readv (int __fd, __const struct iovec *__vector,
			     int __count);
extern ssize_t __writev (int __fd, __const struct iovec *__vector,
			 int __count);
extern ssize_t __libc_writev (int __fd, __const struct iovec *__vector,
			      int __count);
#endif
