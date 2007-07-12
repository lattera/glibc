/* Write formatted list with names for addresses in backtrace to a file.
   Copyright (C) 1998, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <execinfo.h>
#include <string.h>
#include <sys/uio.h>

#include <stdio-common/_itoa.h>
#include <not-cancel.h>

#if __ELF_NATIVE_CLASS == 32
# define WORD_WIDTH 8
#else
/* We assume 64bits.  */
# define WORD_WIDTH 16
#endif


void
__backtrace_symbols_fd (array, size, fd)
     void *const *array;
     int size;
     int fd;
{
  struct iovec iov[3];
  int cnt;

  for (cnt = 0; cnt < size; ++cnt)
    {
      char buf[WORD_WIDTH];

      iov[0].iov_base = (void *) "[0x";
      iov[0].iov_len = 3;

      iov[1].iov_base = _itoa_word ((unsigned long int) array[cnt],
				    &buf[WORD_WIDTH], 16, 0);
      iov[1].iov_len = &buf[WORD_WIDTH] - (char *) iov[1].iov_base;

      iov[2].iov_base = (void *) "]\n";
      iov[2].iov_len = 2;

      /* We prefer to use the non-cancelable interface if it is available.  */
      writev_not_cancel_no_status (fd, iov, 3);
    }
}
weak_alias (__backtrace_symbols_fd, backtrace_symbols_fd)
libc_hidden_def (__backtrace_symbols_fd)
