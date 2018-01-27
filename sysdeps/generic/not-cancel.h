/* Uncancelable versions of cancelable interfaces.  Generic version.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#ifndef NOT_CANCEL_H
# define NOT_CANCEL_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/uio.h>

/* By default we have none.  Map the name to the normal functions.  */
#define __open_nocancel(...) \
  __open (__VA_ARGS__)
#define __open64_nocancel(...) \
  __open64 (__VA_ARGS__)
#define __openat_nocancel(...) \
  __openat (__VA_ARGS__)
#define __openat64_nocancel(...) \
  __openat64 (__VA_ARGS__)
#define __close_nocancel(fd) \
  __close (fd)
#define __close_nocancel_nostatus(fd) \
  (void) __close (fd)
#define __read_nocancel(fd, buf, n) \
  __read (fd, buf, n)
#define __write_nocancel(fd, buf, n) \
  __write (fd, buf, n)
#define __writev_nocancel_nostatus(fd, iov, n) \
  (void) __writev (fd, iov, n)
# define __waitpid_nocancel(pid, stat_loc, options) \
  __waitpid (pid, stat_loc, options)
#define __pause_nocancel() \
  __pause ()
#define __nanosleep_nocancel(requested_time, remaining) \
  __nanosleep (requested_time, remaining)
#define __fcntl_nocancel(fd, cmd, ...) \
  __fcntl (fd, cmd, __VA_ARGS__)

#endif /* NOT_CANCEL_H  */
