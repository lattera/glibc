/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <signal.h>
#include <pthreadP.h>
#include <tls.h>
#include <sysdep.h>


int
pthread_sigmask (how, newmask, oldmask)
     int how;
     const sigset_t *newmask;
     sigset_t *oldmask;
{
  int result = INTERNAL_SYSCALL (sigprocmask, 3, how, newmask, oldmask);

  return (INTERNAL_SYSCALL_ERROR_P (result)
	  ? INTERNAL_SYSCALL_ERRNO (result)
	  : 0);
}
