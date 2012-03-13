/* Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <kernel-features.h>
#include <sysdep-cancel.h>
#include <not-cancel.h>


#ifndef OPENAT
# define OPENAT openat
# define __OPENAT_2 __openat_2

# ifndef __ASSUME_ATFCTS
/* Set errno after a failed call.  If BUF is not null,
   it is a /proc/self/fd/ path name we just tried to use.  */
void
attribute_hidden
__atfct_seterrno (int errval, int fd, const char *buf)
{
  if (buf != NULL)
    {
      struct stat64 st;

      if (errval == ENOTDIR || errval == ENOENT)
	{
	  /* This can mean either the file descriptor is invalid or
	     /proc is not mounted.  */
	  if (__fxstat64 (_STAT_VER, fd, &st) != 0)
	    /* errno is already set correctly.  */
	    return;

	  /* If /proc is not mounted there is nothing we can do.  */
	  if ((errval != ENOTDIR || S_ISDIR (st.st_mode))
	      && (__xstat64 (_STAT_VER, "/proc/self/fd", &st) != 0
		  || !S_ISDIR (st.st_mode)))
	    errval = ENOSYS;
	}
    }

  __set_errno (errval);
}

int __have_atfcts;
# endif
#endif


#define OPENAT_NOT_CANCEL CONCAT (OPENAT)
#define CONCAT(name) CONCAT2 (name)
#define CONCAT2(name) __##name##_nocancel


int
OPENAT_NOT_CANCEL (fd, file, oflag, mode)
     int fd;
     const char *file;
     int oflag;
     mode_t mode;
{

  /* We have to add the O_LARGEFILE flag for openat64.  */
#ifdef MORE_OFLAGS
  oflag |= MORE_OFLAGS;
#endif

  int res;

#ifdef __NR_openat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      res = INLINE_SYSCALL (openat, 4, fd, file, oflag, mode);

# ifndef __ASSUME_ATFCTS
      if (res == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return res;
    }
#endif

#ifndef __ASSUME_ATFCTS
  INTERNAL_SYSCALL_DECL (err);
  char *buf = NULL;

  if (fd != AT_FDCWD && file[0] != '/')
    {
      size_t filelen = strlen (file);
      if (__builtin_expect (filelen == 0, 0))
	{
	  __set_errno (ENOENT);
	  return -1;
	}

      static const char procfd[] = "/proc/self/fd/%d/%s";
      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      buf = alloca (buflen);

      /* Note: snprintf cannot be canceled.  */
      __snprintf (buf, buflen, procfd, fd, file);
      file = buf;
    }

  res = INTERNAL_SYSCALL (open, err, 3, file, oflag, mode);

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (res, err), 0))
    {
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (res, err), fd, buf);
      res = -1;
    }

  return res;
#endif
}

#define UNDERIZE(name) UNDERIZE_1 (name)
#define UNDERIZE_1(name) __##name
#define __OPENAT UNDERIZE (OPENAT)


/* Open FILE with access OFLAG.  Interpret relative paths relative to
   the directory associated with FD.  If OFLAG includes O_CREAT, a
   third argument is the file protection.  */
int
__OPENAT (fd, file, oflag)
     int fd;
     const char *file;
     int oflag;
{
  mode_t mode = 0;
  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
    }

  if (SINGLE_THREAD_P)
    return OPENAT_NOT_CANCEL (fd, file, oflag, mode);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int res = OPENAT_NOT_CANCEL (fd, file, oflag, mode);

  LIBC_CANCEL_RESET (oldtype);

  return res;
}
libc_hidden_def (__OPENAT)
weak_alias (__OPENAT, OPENAT)


int
__OPENAT_2 (fd, file, oflag)
     int fd;
     const char *file;
     int oflag;
{
  if (oflag & O_CREAT)
#define MSG(s) MSG2 (s)
#define MSG2(s) "invalid " #s " call: O_CREAT without mode"
    __fortify_fail (MSG (OPENAT));

  return __OPENAT (fd, file, oflag);
}
