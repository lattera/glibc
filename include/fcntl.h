#ifndef _FCNTL_H
#include <io/fcntl.h>

/* Now define the internal interfaces.  */
extern int __open64 (__const char *__file, int __oflag, ...);
extern int __libc_open64 (const char *file, int oflag, ...);
extern int __libc_open (const char *file, int oflag, ...);
extern int __libc_fcntl (int fd, int cmd, ...);
extern int __open (__const char *__file, int __oflag, ...);
extern int __fcntl (int __fd, int __cmd, ...);
extern int __fcntl_internal (int __fd, int __cmd, ...);

#ifndef NOT_IN_libc
# define __fcntl(fd, cmd, args...) INTUSE(__fcntl) (fd, cmd, ##args)
#endif

#endif
