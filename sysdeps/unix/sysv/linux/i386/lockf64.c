/* Copyright (C) 1994,1996,1997,1998,1999,2000,2003,2006
	Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sysdep.h>

#include <kernel-features.h>

/* lockf is a simplified interface to fcntl's locking facilities.  */

#ifdef __NR_fcntl64
# if __ASSUME_FCNTL64 == 0
/* This variable is shared with all files that check for fcntl64. The
   declaration is in fcntl.c.  */
extern int __have_no_fcntl64;
# endif
#endif

int
lockf64 (int fd, int cmd, off64_t len64)
{
#if __ASSUME_FCNTL64 == 0
  struct flock fl;
  off_t len = (off_t) len64;
#endif
#ifdef __NR_fcntl64
  struct flock64 fl64;
  int cmd64;
#endif

#if __ASSUME_FCNTL64 == 0
  memset ((char *) &fl, '\0', sizeof (fl));

  /* lockf is always relative to the current file position.  */
  fl.l_whence = SEEK_CUR;
  fl.l_start = 0;
  fl.l_len = len;
#endif
#ifdef __NR_fcntl64
# if __ASSUME_FCNTL64 == 0
  if (!__have_no_fcntl64)
    {
# endif
      memset ((char *) &fl64, '\0', sizeof (fl64));
      fl64.l_whence = SEEK_CUR;
      fl64.l_start = 0;
      fl64.l_len = len64;
# if __ASSUME_FCNTL64 == 0
    }
# endif
#endif

#if __ASSUME_FCNTL64 == 0 && !defined __NR_fcntl64
  if (len64 != (off64_t) len)
    {
      /* We can't represent the length.  */
      __set_errno (EOVERFLOW);
      return -1;
    }
#endif
  switch (cmd)
    {
    case F_TEST:
      /* Test the lock: return 0 if FD is unlocked or locked by this process;
	 return -1, set errno to EACCES, if another process holds the lock.  */
#if __ASSUME_FCNTL64 > 0
      fl64.l_type = F_RDLCK;
      if (INLINE_SYSCALL (fcntl64, 3, fd, F_GETLK64, &fl64) < 0)
        return -1;
      if (fl64.l_type == F_UNLCK || fl64.l_pid == __getpid ())
        return 0;
      __set_errno (EACCES);
      return -1;
#else
# ifdef __NR_fcntl64
      if (!__have_no_fcntl64)
	{
	  int res;

	  fl64.l_type = F_RDLCK;
	  res = INLINE_SYSCALL (fcntl64, 3, fd, F_GETLK64, &fl64);
	  /* If errno == ENOSYS try the 32bit interface if len64 can
             be represented with 32 bits.  */

	  if (res == 0)
	    {
	      if (fl64.l_type == F_UNLCK || fl64.l_pid == __getpid ())
		return 0;
	      __set_errno (EACCES);
	      return -1;
	    }
	  else if (errno == ENOSYS)
	    __have_no_fcntl64 = 1;
	  else
	    /* res < 0 && errno != ENOSYS.  */
	    return -1;
	  if (len64 != (off64_t) len)
	    {
	      /* We can't represent the length.  */
	      __set_errno (EOVERFLOW);
	      return -1;
	    }
	}
# endif
      fl.l_type = F_RDLCK;
      if (__fcntl (fd, F_GETLK, &fl) < 0)
	return -1;
      if (fl.l_type == F_UNLCK || fl.l_pid == __getpid ())
	return 0;
      __set_errno (EACCES);
      return -1;
#endif
    case F_ULOCK:
#if __ASSUME_FCNTL64 == 0
      fl.l_type = F_UNLCK;
      cmd = F_SETLK;
#endif
#ifdef __NR_fcntl64
      fl64.l_type = F_UNLCK;
      cmd64 = F_SETLK64;
#endif
      break;
    case F_LOCK:
#if __ASSUME_FCNTL64 == 0
      fl.l_type = F_WRLCK;
      cmd = F_SETLKW;
#endif
#ifdef __NR_fcntl64
      fl64.l_type = F_WRLCK;
      cmd64 = F_SETLKW64;
#endif
      break;
    case F_TLOCK:
#if __ASSUME_FCNTL64 == 0
      fl.l_type = F_WRLCK;
      cmd = F_SETLK;
#endif
#ifdef __NR_fcntl64
      fl64.l_type = F_WRLCK;
      cmd64 = F_SETLK64;
#endif
      break;

    default:
      __set_errno (EINVAL);
      return -1;
    }
#if __ASSUME_FCNTL64 > 0
  return INLINE_SYSCALL (fcntl64, 3, fd, cmd64, &fl64);
#else
# ifdef __NR_fcntl64

  if (!__have_no_fcntl64)
    {
      int res = INLINE_SYSCALL (fcntl64, 3, fd, cmd64, &fl64);

      /* If errno == ENOSYS try the 32bit interface if len64 can
	 be represented with 32 bits.  */
      if (res == 0 || errno != ENOSYS)
	return res;

      __have_no_fcntl64 = 1;

      if (len64 != (off64_t) len)
	{
	  /* We can't represent the length.  */
	  __set_errno (EOVERFLOW);
	  return -1;
	}
    }
# endif
  return __fcntl (fd, cmd, &fl);
#endif
}
