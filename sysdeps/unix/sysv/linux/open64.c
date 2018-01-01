/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include <sysdep-cancel.h>
#include <not-cancel.h>

#ifdef __OFF_T_MATCHES_OFF64_T
# define EXTRA_OPEN_FLAGS 0
#else
# define EXTRA_OPEN_FLAGS O_LARGEFILE
#endif

/* Open FILE with access OFLAG.  If O_CREAT or O_TMPFILE is in OFLAG,
   a third argument is the file protection.  */
int
__libc_open64 (const char *file, int oflag, ...)
{
  int mode = 0;

  if (__OPEN_NEEDS_MODE (oflag))
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  return SYSCALL_CANCEL (openat, AT_FDCWD, file, oflag | EXTRA_OPEN_FLAGS,
			 mode);
}

strong_alias (__libc_open64, __open64)
libc_hidden_weak (__open64)
weak_alias (__libc_open64, open64)

# if !IS_IN (rtld)
int
__open64_nocancel (const char *file, int oflag, ...)
{
  int mode = 0;

  if (__OPEN_NEEDS_MODE (oflag))
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  return INLINE_SYSCALL_CALL (openat, AT_FDCWD, file, oflag | EXTRA_OPEN_FLAGS,
			      mode);
}
#else
strong_alias (__libc_open64, __open64_nocancel)
#endif
libc_hidden_def (__open64_nocancel)

#ifdef __OFF_T_MATCHES_OFF64_T
strong_alias (__libc_open64, __libc_open)
strong_alias (__libc_open64, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open64, open)

strong_alias (__open64_nocancel, __open_nocancel)
libc_hidden_weak (__open_nocancel)
#endif
