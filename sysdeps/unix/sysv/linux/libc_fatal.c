/* Copyright (C) 1993,1994,1995,1997,2000,2002 Free Software Foundation, Inc.
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
#include <abort-instr.h>
#ifndef ABORT_INSTRUCTION
/* No such instruction is available.  */
# define ABORT_INSTRUCTION
#endif

/* Abort with an error message.  */
void
__libc_fatal (message)
     const char *message;
{
  size_t len = strlen (message);

  while (len > 0)
    {
      ssize_t count = INLINE_SYSCALL (write, 3, STDERR_FILENO, message, len);
      if (count > 0)
	{
	  message += count;
	  len -= count;
	}
      else if (count < 0 && errno != EINTR)
	break;
    }

  /* Terminate the process.  */
  _exit (127);

  /* The previous call should never have returned.  */
  while (1)
    /* Try for ever and ever.  */
    ABORT_INSTRUCTION;
}
