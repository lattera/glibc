/* Copyright (C) 2000, 2002, 2003 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <paths.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

/* Try to get a machine dependent instruction which will make the
   program crash.  This is used in case everything else fails.  */
#include <abort-instr.h>
#ifndef ABORT_INSTRUCTION
/* No such instruction is available.  */
# define ABORT_INSTRUCTION
#endif

#include <device-nrs.h>
#include <not-cancel.h>


/* Should other OSes (e.g., Hurd) have different versions which can
   be written in a better way?  */
static void
check_one_fd (int fd, int mode)
{
  /* Note that fcntl() with this parameter is not a cancellation point.  */
  if (__builtin_expect (__libc_fcntl (fd, F_GETFD), 0) == -1
      && errno == EBADF)
    {
      struct stat64 st;

      /* Something is wrong with this descriptor, it's probably not
	 opened.  Open /dev/null so that the SUID program we are
	 about to start does not accidently use this descriptor.  */
      int nullfd = open_not_cancel (_PATH_DEVNULL, mode, 0);
      /* We are very paranoid here.  With all means we try to ensure
	 that we are actually opening the /dev/null device and nothing
	 else.

	 Note that the following code assumes that STDIN_FILENO,
	 STDOUT_FILENO, STDERR_FILENO are the three lowest file
	 decsriptor numbers, in this order.  */
      if (__builtin_expect (nullfd != fd, 0)
	  || __builtin_expect (__fxstat64 (_STAT_VER, fd, &st), 0) != 0
	  || __builtin_expect (S_ISCHR (st.st_mode), 1) == 0
#if defined DEV_NULL_MAJOR && defined DEV_NULL_MINOR
	  || st.st_rdev != makedev (DEV_NULL_MAJOR, DEV_NULL_MINOR)
#endif
	  )
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
  /* This is really paranoid but some people actually are.  If /dev/null
     should happen to be a symlink to somewhere else and not the device
     commonly known as "/dev/null" we bail out.  We can detect this with
     the O_NOFOLLOW flag for open() but only on some system.  */
#ifndef O_NOFOLLOW
# define O_NOFOLLOW	0
#endif
  /* Check all three standard file descriptors.  */
  check_one_fd (STDIN_FILENO, O_RDONLY | O_NOFOLLOW);
  check_one_fd (STDOUT_FILENO, O_RDWR | O_NOFOLLOW);
  check_one_fd (STDERR_FILENO, O_RDWR | O_NOFOLLOW);
}
