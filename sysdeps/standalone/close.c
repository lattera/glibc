/* Copyright (C) 1994 Free Software Foundation, Inc.
   Ported to standalone by Joel Sherrill jsherril@redstone-emh2.army.mil,
     On-Line Applications Research Corporation.

This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio_lim.h>
#include "filedesc.h"

/* Close the file descriptor FD.  */
int
DEFUN(__close, (fd), int fd)
{
  if ( !__FD_Is_valid( fd ) || !__FD_Table[ fd ].in_use )
    {
      errno = EBADF;
      return -1;
    }

  __FD_Table[ fd ].in_use = 0;
  return 0;
}

