/* Copyright (C) 1998-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <paths.h>
#include <sys/statfs.h>

#include "linux_fsinfo.h"

/* Path to the master pseudo terminal cloning device.  */
#define _PATH_DEVPTMX _PATH_DEV "ptmx"
/* Directory containing the UNIX98 pseudo terminals.  */
#define _PATH_DEVPTS _PATH_DEV "pts"

/* Prototype for function that opens BSD-style master pseudo-terminals.  */
extern int __bsd_getpt (void) attribute_hidden;

/* Open a master pseudo terminal and return its file descriptor.  */
int
__posix_openpt (int oflag)
{
  static int have_no_dev_ptmx;
  int fd;

  if (!have_no_dev_ptmx)
    {
      fd = __open (_PATH_DEVPTMX, oflag);
      if (fd != -1)
	{
	  struct statfs fsbuf;
	  static int devpts_mounted;

	  /* Check that the /dev/pts filesystem is mounted
	     or if /dev is a devfs filesystem (this implies /dev/pts).  */
	  if (devpts_mounted
	      || (__statfs (_PATH_DEVPTS, &fsbuf) == 0
		  && fsbuf.f_type == DEVPTS_SUPER_MAGIC)
	      || (__statfs (_PATH_DEV, &fsbuf) == 0
		  && fsbuf.f_type == DEVFS_SUPER_MAGIC))
	    {
	      /* Everything is ok.  */
	      devpts_mounted = 1;
	      return fd;
	    }

	  /* If /dev/pts is not mounted then the UNIX98 pseudo terminals
	     are not usable.  */
	  __close (fd);
	  have_no_dev_ptmx = 1;
	  __set_errno (ENOENT);
	}
      else
	{
	  if (errno == ENOENT || errno == ENODEV)
	    have_no_dev_ptmx = 1;
	  else
	    return -1;
	}
    }
  else
    __set_errno (ENOENT);

  return -1;
}
weak_alias (__posix_openpt, posix_openpt)


int
__getpt (void)
{
  int fd = __posix_openpt (O_RDWR);
  if (fd == -1)
    fd = __bsd_getpt ();
  return fd;
}


#define PTYNAME1 "pqrstuvwxyzabcde";
#define PTYNAME2 "0123456789abcdef";

#define __getpt __bsd_getpt
#define HAVE_POSIX_OPENPT
#include <sysdeps/unix/bsd/getpt.c>
