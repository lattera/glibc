/* Checking macros for unistd functions.
   Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _UNISTD_H
# error "Never include <bits/unistd.h> directly; use <unistd.h> instead."
#endif

extern ssize_t __read_chk (int __fd, void *__buf, size_t __nbytes,
			   size_t __buflen) __wur;
#define read(fd, buf, nbytes) \
  (__bos (buf) != (size_t) -1						      \
   ? __read_chk (fd, buf, nbytes, __bos (buf))				      \
   : read (fd, buf, nbytes))

#ifdef __USE_UNIX98
extern ssize_t __pread_chk (int __fd, void *__buf, size_t __nbytes,
			    __off_t __offset, size_t __bufsize) __wur;
extern ssize_t __pread64_chk (int __fd, void *__buf, size_t __nbytes,
			      __off64_t __offset, size_t __bufsize) __wur;
# ifndef __USE_FILE_OFFSET64
#  define pread(fd, buf, nbytes, offset) \
  (__bos (buf) != (size_t) -1						      \
   ? __pread64_chk (fd, buf, nbytes, offset, __bos (buf))		      \
   : pread (fd, buf, offset, nbytes))
# else
#  define pread(fd, buf, nbytes, offset) \
  (__bos (buf) != (size_t) -1						      \
   ? __pread_chk (fd, buf, nbytes, offset, __bos (buf))			      \
   : pread (fd, buf, offset, nbytes))
# endif

# ifdef __USE_LARGEFILE64
#  define pread64(fd, buf, nbytes, offset) \
  (__bos (buf) != (size_t) -1						      \
   ? __pread64_chk (fd, buf, nbytes, offset, __bos (buf))		      \
   : pread64 (fd, buf, offset, nbytes))
# endif
#endif

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K
extern int __readlink_chk (__const char *__restrict __path,
			   char *__restrict __buf, size_t __len,
			   size_t __buflen)
     __THROW __nonnull ((1, 2)) __wur;
# define readlink(path, buf, len) \
  (__bos (buf) != (size_t) -1						      \
   ? __readlink_chk (path, buf, len, __bos (buf))			      \
   : readlink (path, buf, len))
#endif

extern char *__getcwd_chk (char *__buf, size_t __size, size_t __buflen)
     __THROW __wur;
#define getcwd(buf, size) \
  (__bos (buf) != (size_t) -1						      \
   ? __getcwd_chk (buf, size, buflen) : getcwd (buf, size))

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern char *__getwd_chk (char *__buf, size_t buflen)
     __THROW __nonnull ((1)) __attribute_deprecated__ __wur;
#define getwd(buf) \
  (__bos (buf) != (size_t) -1 ? __getwd_chk (buf, buflen) : getwd (buf))
#endif
