/* Copyright (C) 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <unistd.h>

/* Try to get a machine dependent instruction which will make the
   program crash.  This is used in case everything else fails.  */
#include <abort-instr.h>
#ifndef ABORT_INSTRUCTION
/* No such instruction is available.  */
# define ABORT_INSTRUCTION
#endif


/* Should other OSes (e.g., Hurd) have different versions which can
   be written in a better way?  */
static void
check_one_fd (int fd, int mode)
{
  if (__builtin_expect (__libc_fcntl (fd, F_GETFD), 0) == -1
      && errno == EBADF)
    {
      /* Something is wrong with this descriptor, it's probably not
	 opened.  Open /dev/null so that the SUID program we are
	 about to start does not accidently use this descriptor.  */
      int nullfd = __libc_open (_PATH_DEVNULL, mode);
      if (__builtin_expect (nullfd, 0) == -1)
	/* We cannot even give an error message here since it would
	   run into the same problems.  */
	while (1)
	  /* Try for ever and ever.  */
	  ABORT_INSTRUCTION;
    }
}


void
__libc_check_standard_fds (void)
{
/* Check all three standard file descriptors.  */
  check_one_fd (STDIN_FILENO, O_RDONLY);
  check_one_fd (STDOUT_FILENO, O_RDWR);
  check_one_fd (STDERR_FILENO, O_RDWR);
}
