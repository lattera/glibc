#ifndef _SYS_STAT_H
#include <io/sys/stat.h>

/* Now define the internal interfaces. */
extern int __stat __P ((__const char *__file, struct stat *__buf));
extern int __fstat __P ((int __fd, struct stat *__buf));
extern int __lstat __P ((__const char *__file, struct stat *__buf));
extern int __chmod __P ((__const char *__file, __mode_t __mode));
extern int __fchmod __P ((int __fd, __mode_t __mode));
extern __mode_t __umask __P ((__mode_t __mask));
extern int __mkdir __P ((__const char *__path, __mode_t __mode));
extern int __mknod __P ((__const char *__path,
			 __mode_t __mode, __dev_t __dev));
extern __inline__ int __stat (__const char *__path, struct stat *__statbuf)
{
  return __xstat (_STAT_VER, __path, __statbuf);
}
extern __inline__ int __lstat (__const char *__path, struct stat *__statbuf)
{
  return __lxstat (_STAT_VER, __path, __statbuf);
}
extern __inline__ int __fstat (int __fd, struct stat *__statbuf)
{
  return __fxstat (_STAT_VER, __fd, __statbuf);
}
extern __inline__ int __mknod (__const char *__path, __mode_t __mode,
			       __dev_t __dev)
{
  return __xmknod (_MKNOD_VER, __path, __mode, &__dev);
}


/* The `stat', `fstat', `lstat' functions have to be handled special since
   even while not compiling the library with optimization calls to these
   functions in the shared library must reference the `xstat' etc functions.
   We have to use macros but we cannot define them in the normal headers
   since on user level we must use real functions.  */
#define stat(fname, buf) __xstat (_STAT_VER, fname, buf)
#define fstat(fd, buf) __fxstat (_STAT_VER, fd, buf)
#define __fstat(fd, buf)  __fxstat (_STAT_VER, fd, buf)
#define lstat(fname, buf)  __lxstat (_STAT_VER, fname, buf)
#define __lstat(fname, buf)  __lxstat (_STAT_VER, fname, buf)
#define stat64(fname, buf) __xstat64 (_STAT_VER, fname, buf)
#define fstat64(fd, buf) __fxstat64 (_STAT_VER, fd, buf)
#define lstat64(fname, buf)  __lxstat64 (_STAT_VER, fname, buf)
#endif
