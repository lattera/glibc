#ifndef _SYS_UIO_H
#include <misc/sys/uio.h>

#ifndef _ISOMAC
/* Now define the internal interfaces.  */
extern ssize_t __readv (int __fd, const struct iovec *__iovec,
			int __count);
extern ssize_t __writev (int __fd, const struct iovec *__iovec,
			 int __count);
#endif
#endif
