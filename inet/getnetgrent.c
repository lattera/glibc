/* Copyright (C) 1996, 1997, 2000, 2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <bits/libc-lock.h>

/* Static buffer for return value.  We allocate it when needed.  */
libc_freeres_ptr (static char *buffer);
/* All three strings should fit in a block of 1kB size.  */
#define BUFSIZE 1024



static void
allocate (void)
{
  buffer = (char *) malloc (BUFSIZE);
}

int
getnetgrent (char **hostp, char **userp, char **domainp)
{
  __libc_once_define (static, once);
  __libc_once (once, allocate);

  if (buffer == NULL)
    {
      __set_errno (ENOMEM);
      return -1;
    }

  return __getnetgrent_r (hostp, userp, domainp, buffer, BUFSIZE);
}
