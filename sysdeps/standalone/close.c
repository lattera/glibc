/* Copyright (C) 1994, 1995, 1996, 1997, 2002 Free Software Foundation, Inc.
   Ported to standalone by Joel Sherrill jsherril@redstone-emh2.army.mil,
     On-Line Applications Research Corporation.
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
#include <unistd.h>

#define _STDIO_H
#include <bits/stdio_lim.h>
#include "filedesc.h"

/* Close the file descriptor FD.  */
int
__close (fd)
     int fd;
{
  if ( !__FD_Is_valid( fd ) || !__FD_Table[ fd ].in_use )
    {
      __set_errno (EBADF);
      return -1;
    }

  __FD_Table[ fd ].in_use = 0;
  return 0;
}
libc_hidden_def (__close)
weak_alias (__close, close)
