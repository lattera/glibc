#ifndef _FCNTL_H
#include <io/fcntl.h>

/* Now define the internal interfaces.  */
extern int __open64 (__const char *__file, int __oflag, ...);
libc_hidden_proto (__open64)
extern int __libc_open64 (const char *file, int oflag, ...);
extern int __libc_open (const char *file, int oflag, ...);
libc_hidden_proto (__libc_open)
extern int __libc_creat (const char *file, mode_t mode);
extern int __libc_fcntl (int fd, int cmd, ...);
#ifndef NO_CANCELLATION
extern int __fcntl_nocancel (int fd, int cmd, ...) attribute_hidden;
libc_hidden_proto (__libc_fcntl)
#endif
extern int __open (__const char *__file, int __oflag, ...);
libc_hidden_proto (__open)
extern int __fcntl (int __fd, int __cmd, ...);
libc_hidden_proto (__fcntl)
extern int __openat (int __fd, __const char *__file, int __oflag, ...)
  __nonnull ((2));
libc_hidden_proto (__openat)
extern int __openat64 (int __fd, __const char *__file, int __oflag, ...)
  __nonnull ((2));
libc_hidden_proto (__openat64)

extern int __open_2 (__const char *__path, int __oflag);
extern int __open64_2 (__const char *__path, int __oflag);
extern int __openat_2 (int __fd, __const char *__path, int __oflag);
extern int __openat64_2 (int __fd, __const char *__path, int __oflag);


/* Helper functions for the various *at functions.  For Linux.  */
extern void __atfct_seterrno (int errval, int fd, const char *buf)
  attribute_hidden;
extern void __atfct_seterrno_2 (int errval, int fd1, const char *buf1,
				int fd2, const char *buf2)
  attribute_hidden;


/* Flag determining whether the *at system calls are available.  */
extern int __have_atfcts attribute_hidden;

#ifdef O_CLOEXEC
extern int __have_o_cloexec attribute_hidden;
#endif

#endif
