/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

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
#include <stdlib.h>

/* Path to the master pseudo terminal cloning device.  */
#define _PATH_DEVPTMX "/dev/ptmx"

/* Prototype for function that opens BSD-style master pseudo-terminals.  */
int __bsd_getpt (void);

/* Open a master pseudo terminal and return its file descriptor.  */
int
__getpt (void)
{
  static int have_dev_ptmx = 1;
  int fd;

  if (have_dev_ptmx)
    {
      fd = __open (_PATH_DEVPTMX, O_RDWR);
      if (fd != -1)
	return fd;
      else
	{
	  if (errno == ENOENT || errno == ENODEV)
	    have_dev_ptmx = 0;
	  else
	    return -1;
	}
    }

  return __bsd_getpt ();
}

#define PTYNAME1 "pqrstuvwxyzabcde";
#define PTYNAME2 "0123456789abcdef";

#define __getpt __bsd_getpt
#include <sysdeps/unix/bsd/getpt.c>
