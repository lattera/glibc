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
#include <pthreadP.h>


int
pthread_mutexattr_setpshared (attr, pshared)
     pthread_mutexattr_t *attr;
     int pshared;
{
  struct pthread_mutexattr *iattr;

  if (pshared != PTHREAD_PROCESS_PRIVATE
      && __builtin_expect (pshared != PTHREAD_PROCESS_SHARED, 0))
    return EINVAL;

  iattr = (struct pthread_mutexattr *) attr;

  /* We use bit 31 to signal whether the mutex is going to be
     process-shared or not.  */
  if (pshared == PTHREAD_PROCESS_PRIVATE)
    iattr->mutexkind &= ~0x80000000;
  else
    iattr->mutexkind |= 0x80000000;

  return 0;
}
