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
#include <stddef.h>

#include "filedesc.h"
#include <fcntl.h>
#include <standalone.h>

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
ssize_t
DEFUN(__read, (fd, buf, nbytes),
      int fd AND PTR buf AND size_t nbytes)
{
  char *buffer = (char *) buf;
  int data;
  int poll;

  errno = 0;

  if (nbytes == 0)
    return 0;

  if ( !__FD_Is_valid( fd ) || !__FD_Table[ fd ].in_use )
    {
      errno = EBADF;
      return -1;
    }
  if (buf == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  if ( __FD_Table[ fd ].flags & O_WRONLY )  /* is it write only? */
    {
      errno = EBADF;
      return -1;
    }

  /* If this is a non-blocking fd, then we want to poll the console.  */

  poll = ( __FD_Table[ fd ].flags & O_NONBLOCK ) ? 1 : 0;

  /* Read a single character.  This is a cheap way to insure that the 
     upper layers get every character because _Console_Getc can't timeout 
     or otherwise know when to stop.  */

  
  data = _Console_Getc(poll);

  if ( data == -1 )                 /* if no data return */
    return -1;

  (void) _Console_Putc(data);       /* echo the character */

  if ( data == '\r' ) {		/* translate CR -> CR/LF */
    (void) _Console_Putc('\n');
    data = '\n';
  }

  *buffer = data;
  return 1;
}
