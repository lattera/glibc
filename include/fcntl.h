#ifndef _FCNTL_H
#include <io/fcntl.h>

/* Now define the internal interfaces.  */
extern int __open64 (__const char *__file, int __oflag, ...);
extern int __libc_open64 (const char *file, int oflag, ...);
extern int __libc_open (const char *file, int oflag, ...);
extern int __libc_fcntl (int fd, int cmd, ...);
extern int __open (__const char *__file, int __oflag, ...);
extern int __fcntl (int __fd, int __cmd, ...) __THROW;

#endif
