#ifndef _FCNTL_H
#include <io/fcntl.h>

/* Now define the internal interfaces.  */
extern int __open64 (__const char *__file, int __oflag, ...);
extern int __libc_open64 (const char *file, int oflag, ...);
extern int __libc_open (const char *file, int oflag, ...);
extern int __libc_fcntl (int fd, int cmd, ...);
extern int __open (__const char *__file, int __oflag, ...);
extern int __open_internal (__const char *__file, int __oflag, ...)
     attribute_hidden;
extern int __fcntl (int __fd, int __cmd, ...);
extern int __fcntl_internal (int __fd, int __cmd, ...) attribute_hidden;

#ifndef NOT_IN_libc
# define __fcntl(fd, cmd, args...) INTUSE(__fcntl) (fd, cmd, ##args)
# define __open(file, oflag, args...) INTUSE(__open) (file, oflag, ##args)
# ifdef SHARED
#  define __libc_fcntl(fd, cmd, args...) __fcntl_internal (fd, cmd, ##args)
#  define __libc_open(file, oflag, args...) \
  __open_internal (file, oflag, ##args)
# endif
#endif

#endif
