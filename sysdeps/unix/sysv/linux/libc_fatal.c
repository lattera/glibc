/* Catastrophic failure reports.  Linux version.
   Copyright (C) 1993-2018 Free Software Foundation, Inc.
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
#include <execinfo.h>
#include <fcntl.h>
#include <not-cancel.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/uio.h>

static bool
writev_for_fatal (int fd, const struct iovec *iov, size_t niov, size_t total)
{
  INTERNAL_SYSCALL_DECL (err);
  ssize_t cnt;
  do
    cnt = INTERNAL_SYSCALL (writev, err, 3, fd, iov, niov);
  while (INTERNAL_SYSCALL_ERROR_P (cnt, err)
         && INTERNAL_SYSCALL_ERRNO (cnt, err) == EINTR);
  return cnt == total;
}
#define WRITEV_FOR_FATAL	writev_for_fatal

static void
backtrace_and_maps (int do_abort, bool written, int fd)
{
  if (do_abort > 1 && written)
    {
      void *addrs[64];
#define naddrs (sizeof (addrs) / sizeof (addrs[0]))
      int n = __backtrace (addrs, naddrs);
      if (n > 2)
        {
#define strnsize(str) str, strlen (str)
#define writestr(str) __write_nocancel (fd, str)
          writestr (strnsize ("======= Backtrace: =========\n"));
          __backtrace_symbols_fd (addrs + 1, n - 1, fd);

          writestr (strnsize ("======= Memory map: ========\n"));
          int fd2 = __open_nocancel ("/proc/self/maps", O_RDONLY);
          char buf[1024];
          ssize_t n2;
          while ((n2 = __read_nocancel (fd2, buf, sizeof (buf))) > 0)
            if (__write_nocancel (fd, buf, n2) != n2)
              break;
          __close_nocancel_nostatus (fd2);
        }
    }
}
#define BEFORE_ABORT		backtrace_and_maps

#include <sysdeps/posix/libc_fatal.c>
