/* Set source filter.  Stub version.
   Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2004.

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
#include <netinet/in.h>


int
setsourcefilter (int s, uint32_t interface, const struct sockaddr *group,
		 socklen_t grouplen, uint32_t fmode, uint32_t numsrc,
		 const struct sockaddr_storage *slist)
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (setsourcefilter)
