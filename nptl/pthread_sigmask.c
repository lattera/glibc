/* Examine and change blocked signals for a thread.  Generic POSIX version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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
#include <signal.h>
#include <pthreadP.h>

#if defined SIGCANCEL || defined SIGTIMER || defined SIGSETXID
# error "This implementation assumes no internal-only signal numbers."
#endif

int
pthread_sigmask (int how, const sigset_t *newmask, sigset_t *oldmask)
{
  /* Here we assume that sigprocmask actually does everything right.
     The only difference is the return value protocol.  */
  int result = sigprocmask (how, newmask, oldmask);
  if (result < 0)
    result = errno;
  return result;
}
