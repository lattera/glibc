/* Copyright (C) 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <linux/posix_types.h>

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


extern int __getdents __P ((int fd, char *buf, size_t nbytes));

/* For Linux we need a special version of this file since the
   definition of `struct dirent' is not the same for the kernel and
   the libc.  There is one additional field which might be introduced
   in the kernel structure in the future.

   He is the kernel definition of `struct dirent' as of 2.1.20:  */

struct kernel_dirent
  {
    long int d_ino;
    __kernel_off_t d_off;
    unsigned short int d_reclen;
    char d_name[256];
  };


/* The problem here is that we cannot simply read the next NBYTES
   bytes.  We need to take the additional field into account.  We use
   some heuristic.  Assume the directory contains names with at least
   3 characters we can compute a maximum number of entries which fit
   in the buffer.  Taking this number allows us to specify a correct
   number of bytes to read.  If we should be wrong, we can reset the
   file descriptor.  */
int __getdirentries __P ((int fd, char *buf, size_t nbytes, off_t *basep));
int
__getdirentries (fd, buf, nbytes, basep)
     int fd;
     char *buf;
     size_t nbytes;
     off_t *basep;
{
  off_t base = __lseek (fd, (off_t) 0, SEEK_CUR);
  size_t red_nbytes;
  struct kernel_dirent *kdp;
  struct dirent *dp;
  int retval;

  red_nbytes = nbytes - (nbytes / (offsetof (struct dirent, d_name) + 3));

  dp = (struct dirent *) buf;
  kdp = (struct kernel_dirent *) (buf + (nbytes - red_nbytes));

  retval = __getdents (fd, (char *) kdp, red_nbytes);

  while ((char *) kdp < buf + (nbytes - red_nbytes) + retval)
    {
      dp->d_ino = kdp->d_ino;
      dp->d_off = kdp->d_off;
      dp->d_reclen = (kdp->d_reclen
		      + (offsetof (struct dirent, d_name)
			 - offsetof (struct kernel_dirent, d_name)));
      dp->d_type = DT_UNKNOWN;
      memmove (dp->d_name, kdp->d_name,
	       kdp->d_reclen - offsetof (struct kernel_dirent, d_name));

      dp = (struct dirent *) (((char *) dp) + dp->d_reclen);
      kdp = (struct kernel_dirent *) (((char *) kdp) + kdp->d_reclen);

      if ((char *) dp >= (char *) kdp)
	{
	  /* Our heuristic failed.  We read too many entries.  Reset
	     the stream.  */
	  off_t used = ((char *) kdp - (char *) buf) - (nbytes - red_nbytes);
	  base = __lseek (fd, retval - used, SEEK_CUR);
	}
    }

  if (basep)
    *basep = base;

  return (char *) dp - (char *) buf;
}

weak_alias (__getdirentries, getdirentries)
