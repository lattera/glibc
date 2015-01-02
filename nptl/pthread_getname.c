/* pthread_getname_np -- Get thread name.  Stub version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <pthreadP.h>

int
pthread_getname_np (pthread_t th, char *buf, size_t len)
{
  const struct pthread *pd = (const struct pthread *) th;

  if (INVALID_TD_P (pd))
    return ESRCH;

  return ENOSYS;
}
stub_warning (pthread_getname_np)
