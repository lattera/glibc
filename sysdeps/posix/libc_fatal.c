/* Copyright (C) 1993, 1994, 1995, 1997, 2000 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sysdep.h>
#include <string.h>

#ifdef FATAL_PREPARE_INCLUDE
#include FATAL_PREPARE_INCLUDE
#endif

/* Abort with an error message.  */
void
__libc_fatal (message)
     const char *message;
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
