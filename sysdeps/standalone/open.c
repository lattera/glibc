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
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>

#include <stdio.h>
#include <stdio_lim.h>
#include <unistd.h>

#define __DECLARE_FILE_DESCRIPTORS__

#include "filedesc.h"

/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int
DEFUN(__open, (file, oflag), CONST char *file AND int oflag DOTS)
{
  int mode;
  int newfd;
  int index;

  if (file == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start(arg, oflag);
      mode = va_arg(arg, int);
      va_end(arg);
    }

  /*
   *  Find an open slot.
   */

  newfd = -1;

  for ( index=0 ; index< FOPEN_MAX ; index++ )
    if ( !__FD_Table[ index ].in_use ) {
      newfd = index;
      break;
    }

  if ( newfd == -1 ) {
    errno = ENFILE;
    return -1;
  }

  /* 
   *  Initialize the open slot
   */

  __FD_Table[ newfd ].in_use = 1;
  __FD_Table[ newfd ].flags = oflag;
  
  return newfd;
}

/* Initialization Code for Console I/O */

#ifdef HAVE_GNU_LD
static
#endif
void
DEFUN(__NONE_init_console_io, (argc, argv, envp),
      int argc AND char **argv AND char **envp)
{
  int index;

  for ( index=0 ; index< FOPEN_MAX ; index++ )
    __FD_Table[ index ].in_use = 0;

  stdin = fopen( "", "r" );

  stdout = fopen( "", "w" );

  stderr = fopen( "", "w" );

  /*
   *  Line buffer the standard input and output and use no buffering for
   *  standard error.
   */

  setvbuf( stdin,  NULL, _IOLBF, BUFSIZ );
  setvbuf( stdout, NULL, _IOLBF, BUFSIZ );
  setvbuf( stderr, NULL, _IONBF, BUFSIZ );

  (void) &__NONE_init_console_io;  /* Avoid "defined but not used" warning. */
}

#ifdef  HAVE_GNU_LD

#include <gnu-stabs.h>

text_set_element (__libc_subinit, __NONE_init_console_io);

#endif
