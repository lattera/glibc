/* Wrapper arpund system calls to provide cancelation points.
   Copyright (C) 1996-1999,2000-2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/socket.h>


#ifndef SHARED
/* We need a hook to force this file to be linked in when static
   libpthread is used.  */
const int __pthread_provide_wrappers = 0;
#endif


#define CANCELABLE_SYSCALL(res_type, name, param_list, params) \
extern res_type __libc_##name param_list;				      \
res_type								      \
__attribute__ ((weak))							      \
name param_list								      \
{									      \
  res_type result;							      \
  int oldtype;								      \
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);	      \
  result = __libc_##name params;					      \
  pthread_setcanceltype (oldtype, NULL);				      \
  return result;							      \
}

#define CANCELABLE_SYSCALL_VA(res_type, name, param_list, params, last_arg) \
res_type __libc_##name param_list;					      \
res_type								      \
__attribute__ ((weak))							      \
name param_list								      \
{									      \
  res_type result;							      \
  int oldtype;								      \
  va_list ap;								      \
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);	      \
  va_start (ap, last_arg);						      \
  result = __libc_##name params;					      \
  va_end (ap);								      \
  pthread_setcanceltype (oldtype, NULL);				      \
  return result;							      \
}

#define PROMOTE_INTEGRAL_TYPE(type) __typeof__ ((type) 0 + 0)


/* close(2).  */
CANCELABLE_SYSCALL (int, close, (int fd), (fd))
strong_alias (close, __close)


/* creat(2).  */
CANCELABLE_SYSCALL (int, creat, (const char *pathname, mode_t mode),
		    (pathname, mode))


/* fcntl(2).  */
CANCELABLE_SYSCALL_VA (int, fcntl, (int fd, int cmd, ...),
		       (fd, cmd, va_arg (ap, long int)), cmd)
strong_alias (fcntl, __fcntl)


/* fsync(2).  */
CANCELABLE_SYSCALL (int, fsync, (int fd), (fd))


/* lseek(2).  */
CANCELABLE_SYSCALL (off_t, lseek, (int fd, off_t offset, int whence),
		    (fd, offset, whence))
strong_alias (lseek, __lseek)


/* lseek64(2).  */
CANCELABLE_SYSCALL (off64_t, lseek64, (int fd, off64_t offset, int whence),
		    (fd, offset, whence))


/* msync(2).  */
CANCELABLE_SYSCALL (int, msync, (__ptr_t addr, size_t length, int flags),
		    (addr, length, flags))


/* nanosleep(2).  */
CANCELABLE_SYSCALL (int, nanosleep, (const struct timespec *requested_time,
				     struct timespec *remaining),
		    (requested_time, remaining))
strong_alias (nanosleep, __nanosleep)


/* open(2).  */
CANCELABLE_SYSCALL_VA (int, open, (const char *pathname, int flags, ...),
		       (pathname, flags,
			va_arg (ap, PROMOTE_INTEGRAL_TYPE (mode_t))),
		       flags)
strong_alias (open, __open)


/* open64(3).  */
CANCELABLE_SYSCALL_VA (int, open64, (const char *pathname, int flags, ...),
		       (pathname, flags,
			va_arg (ap, PROMOTE_INTEGRAL_TYPE (mode_t))),
		       flags)
strong_alias (open64, __open64)


/* pause(2).  */
CANCELABLE_SYSCALL (int, pause, (void), ())


/* poll(2).  */
CANCELABLE_SYSCALL (int, poll,
		    (struct pollfd *ufds, nfds_t nfds, int timeout),
		    (ufds, nfds, timeout))


/* pread(3).  */
CANCELABLE_SYSCALL (ssize_t, pread, (int fd, void *buf, size_t count,
				     off_t offset),
		    (fd, buf, count, offset))


/* pread64(3).  */
CANCELABLE_SYSCALL (ssize_t, pread64, (int fd, void *buf, size_t count,
				       off64_t offset),
		    (fd, buf, count, offset))
strong_alias (pread64, __pread64)


/* pselect(3).  */
CANCELABLE_SYSCALL (int, pselect, (int n, fd_set *readfds, fd_set *writefds,
				   fd_set *exceptfds,
				   const struct timespec *timeout,
				   const sigset_t *sigmask),
		    (n, readfds, writefds, exceptfds, timeout, sigmask))


/* pwrite(3).  */
CANCELABLE_SYSCALL (ssize_t, pwrite, (int fd, const void *buf, size_t n,
				      off_t offset),
		    (fd, buf, n, offset))


/* pwrite64(3).  */
CANCELABLE_SYSCALL (ssize_t, pwrite64, (int fd, const void *buf, size_t n,
					off64_t offset),
		    (fd, buf, n, offset))
strong_alias (pwrite64, __pwrite64)


/* read(2).  */
CANCELABLE_SYSCALL (ssize_t, read, (int fd, void *buf, size_t count),
		    (fd, buf, count))
strong_alias (read, __read)


/* readv(2).  */
CANCELABLE_SYSCALL (ssize_t, readv,
		    (int fd, const struct iovec *vector, int count),
		    (fd, vector, count))


/* select(2).  */
CANCELABLE_SYSCALL (int, select, (int n, fd_set *readfds,
				  fd_set *writefds,
				  fd_set *exceptfds,
				  struct timeval *timeout),
		    (n, readfds, writefds, exceptfds, timeout))


/* sigpause(3).  */
#undef sigpause
CANCELABLE_SYSCALL (int, sigpause, (int sigmask), (sigmask))


/* __xpg_sigpause(3).  */
CANCELABLE_SYSCALL (int, __xpg_sigpause, (int sigmask), (sigmask))


/* sigsuspend(2).  */
CANCELABLE_SYSCALL (int, sigsuspend, (const sigset_t *mask), (mask))


/* sigwaitinfo(3).  */
CANCELABLE_SYSCALL (int, sigwaitinfo, (const sigset_t *set, siginfo_t *info),
		    (set, info))


/* system(3).  */
CANCELABLE_SYSCALL (int, system, (const char *line), (line))


/* tcdrain(2).  */
CANCELABLE_SYSCALL (int, tcdrain, (int fd), (fd))


/* wait(2).  */
CANCELABLE_SYSCALL (__pid_t, wait, (__WAIT_STATUS_DEFN stat_loc), (stat_loc))
strong_alias (wait, __wait)


/* waitid(3).  */
CANCELABLE_SYSCALL (int, waitid,
		    (idtype_t idtype, id_t id, siginfo_t *info, int options),
		    (idtype, id, info, options))


/* waitpid(2).  */
CANCELABLE_SYSCALL (__pid_t, waitpid, (__pid_t pid, int *stat_loc,
				       int options),
		    (pid, stat_loc, options))


/* write(2).  */
CANCELABLE_SYSCALL (ssize_t, write, (int fd, const void *buf, size_t n),
		    (fd, buf, n))
strong_alias (write, __write)


/* writev(2).  */
CANCELABLE_SYSCALL (ssize_t, writev,
		    (int fd, const struct iovec *vector, int count),
		    (fd, vector, count))


/* The following system calls are thread cancellation points specified
   in XNS.  */

/* accept(2).  */
CANCELABLE_SYSCALL (int, accept, (int fd, __SOCKADDR_ARG addr,
				  socklen_t *addr_len),
		    (fd, addr, addr_len))

/* connect(2).  */
CANCELABLE_SYSCALL (int, connect, (int fd, __CONST_SOCKADDR_ARG addr,
				     socklen_t len),
		    (fd, addr, len))
strong_alias (connect, __connect)

/* recv(2).  */
CANCELABLE_SYSCALL (ssize_t, recv, (int fd, __ptr_t buf, size_t n, int flags),
		    (fd, buf, n, flags))

/* recvfrom(2).  */
CANCELABLE_SYSCALL (ssize_t, recvfrom, (int fd, __ptr_t buf, size_t n, int flags,
					__SOCKADDR_ARG addr, socklen_t *addr_len),
		    (fd, buf, n, flags, addr, addr_len))

/* recvmsg(2).  */
CANCELABLE_SYSCALL (ssize_t, recvmsg, (int fd, struct msghdr *message, int flags),
		    (fd, message, flags))

/* send(2).  */
CANCELABLE_SYSCALL (ssize_t, send, (int fd, const __ptr_t buf, size_t n,
				    int flags),
		    (fd, buf, n, flags))
strong_alias (send, __send)

/* sendmsg(2).  */
CANCELABLE_SYSCALL (ssize_t, sendmsg, (int fd, const struct msghdr *message,
				       int flags),
		    (fd, message, flags))

/* sendto(2).  */
CANCELABLE_SYSCALL (ssize_t, sendto, (int fd, const __ptr_t buf, size_t n,
				      int flags, __CONST_SOCKADDR_ARG addr,
				      socklen_t addr_len),
		    (fd, buf, n, flags, addr, addr_len))
