/* Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sysdep.h>

#ifdef FATAL_PREPARE_INCLUDE
#include FATAL_PREPARE_INCLUDE
#endif

/* Abort with an error message.  */
void
DEFUN(__libc_fatal, (message), CONST char *message)
{
  size_t len = strlen (message);

#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  while (len > 0)
    {
      register int count = __write (STDERR_FILENO, message, len);
      if (count > 0)
	{
	  message += count;
	  len -= count;
	}
      else if (count < 0
#ifdef EINTR
	       && errno != EINTR
#endif
	       )
	break;
    }

  abort ();
}
