/* Copyright (C) 1993-1995,1997,2000,2002, 2003 Free Software Foundation, Inc.
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
      INTERNAL_SYSCALL_DECL (err);
      ssize_t count = INTERNAL_SYSCALL (write, err, 3, STDERR_FILENO,
					message, len);
      if (! INTERNAL_SYSCALL_ERROR_P (count, err))
	{
	  message += count;
	  len -= count;
	}
      else if (INTERNAL_SYSCALL_ERRNO (count, err) != EINTR)
	break;
    }

  /* Terminate the process.  */
  _exit (127);

  /* The previous call should never have returned.  */
  while (1)
    /* Try for ever and ever.  */
    ABORT_INSTRUCTION;
}
libc_hidden_def (__libc_fatal)
