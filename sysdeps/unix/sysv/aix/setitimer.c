/* Copyright (C) 1991, 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

extern int __libc_nanosleep (const struct timespec *requested_time,
			     struct timespec *remaining);
int
__setitimer (which, new, old)
     enum __itimer_which which;
     const struct itimerval *new;
     struct itimerval *old;
{
  if (new == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  switch (which)
   {
    default:
      __set_errno (EINVAL);
      return -1;

    case ITIMER_VIRTUAL:
    case ITIMER_PROF:
      __set_errno (ENOSYS);
      return -1;

    case ITIMER_REAL:
      break;
   }

  switch (__fork())
   {
    case -1: exit(-1);
    case  0:
       {
        struct timespec ts ={.tv_sec = (long int)new->it_value.tv_sec, .tv_nsec = 0};
        __libc_nanosleep(&ts,&ts);
	__kill(getppid(), SIGALRM);
	exit(0);
       }
    default:
   }
  return 0;
}
weak_alias (__setitimer, setitimer)
