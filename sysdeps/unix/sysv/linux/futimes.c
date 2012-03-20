/* futimes -- change access and modification times of open file.  Linux version.
   Copyright (C) 2002,2003,2005,2006,2007,2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <sys/time.h>
#include <_itoa.h>
#include <fcntl.h>

#include <kernel-features.h>


#if defined __NR_utimensat && !defined __ASSUME_UTIMENSAT
static int miss_utimensat;
#endif

/* Change the access time of the file associated with FD to TVP[0] and
   the modification time of FILE to TVP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall which
   can be used to implement futimes.  Earlier kernels have no futimes()
   syscall so we use the /proc filesystem.  */
int
__futimes (int fd, const struct timeval tvp[2])
{
  /* The utimensat system call expects timespec not timeval.  */
  struct timespec ts[2];
  if (tvp != NULL)
    {
      if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
          || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
	{
	  __set_errno (EINVAL);
	  return -1;
	}

      TIMEVAL_TO_TIMESPEC (&tvp[0], &ts[0]);
      TIMEVAL_TO_TIMESPEC (&tvp[1], &ts[1]);
    }

#ifdef __ASSUME_UTIMENSAT
  return INLINE_SYSCALL (utimensat, 4, fd, NULL, tvp ? &ts : NULL, 0);
#else
  int result;
# ifdef __NR_utimensat
  if (!__builtin_expect (miss_utimensat, 0))
    {
      result = INLINE_SYSCALL (utimensat, 4, fd, NULL, tvp ? &ts : NULL, 0);
      if (__builtin_expect (result, 0) != -1 || errno != ENOSYS)
	return result;

      miss_utimensat = 1;
    }
# endif

  static const char selffd[] = "/proc/self/fd/";
  char fname[sizeof (selffd) + 3 * sizeof (int)];
  fname[sizeof (fname) - 1] = '\0';
  char *cp = _itoa_word ((unsigned int) fd, fname + sizeof (fname) - 1, 10, 0);
  cp = memcpy (cp - sizeof (selffd) + 1, selffd, sizeof (selffd) - 1);

# ifdef __NR_utimes
  result = INLINE_SYSCALL (utimes, 2, cp, tvp);
#  ifndef __ASSUME_UTIMES
  if (result == -1 && errno == ENOSYS)
#  endif
# endif
    {
      /* The utimes() syscall does not exist or is not available in the
	 used kernel.  Use utime().  For this we have to convert to the
	 data format utime() expects.  */
# ifndef __ASSUME_UTIMES
      struct utimbuf buf;
      struct utimbuf *times;

      if (tvp != NULL)
	{
	  times = &buf;
	  buf.actime = tvp[0].tv_sec;
	  buf.modtime = tvp[1].tv_sec;
	}
      else
	times = NULL;

      result = INLINE_SYSCALL (utime, 2, cp, times);
# endif
    }

  if (result == -1)
    /* Check for errors that result from failing to find /proc.
       This means we can't do futimes at all, so return ENOSYS
       rather than some confusing error.  */
    switch (errno)
      {
      case EACCES:
	if (tvp == NULL)  /* Could be a path problem or a file problem.  */
	  break;
	/*FALLTHROUGH*/
      case ELOOP:
      case ENAMETOOLONG:
      case ENOTDIR:
	__set_errno (ENOSYS);
	break;

      case ENOENT:
	/* Validate the file descriptor by letting fcntl set errno to
	   EBADF if it's bogus.  Otherwise it's a /proc issue.  */
# if !defined __NR_fcntl && defined __NR_fcntl64
#  define __NR_fcntl __NR_fcntl64
# endif
	if (INLINE_SYSCALL (fcntl, 3, fd, F_GETFD, 0) != -1)
	  __set_errno (ENOSYS);
	break;
      }

  return result;
#endif
}
weak_alias (__futimes, futimes)
