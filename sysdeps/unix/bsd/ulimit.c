/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

#ifndef	 HAVE_GNU_LD
#define	 _etext	etext
#endif

extern int _etext;

/* Function depends on CMD:
   1 = Return the limit on the size of a file, in units of 512 bytes.
   2 = Set the limit on the size of a file to NEWLIMIT.  Only the
       super-user can increase the limit.
   3 = Return the maximum possible address of the data segment.
   4 = Return the maximum number of files that the calling process
       can open.
   Returns -1 on errors.  */
long int
DEFUN(ulimit, (cmd, newlimit),
      int cmd AND long int newlimit)
{
  int status;

  switch (cmd)
    {
    case 1:
      {
	/* Get limit on file size.  */
	struct rlimit fsize;

	status = getrlimit(RLIMIT_FSIZE, &fsize);
	if (status < 0)
	  return -1;

	/* Convert from bytes to 512 byte units.  */
	return fsize.rlim_cur / 512;
      }
    case 2:
      /* Set limit on file size.  */
      {
	struct rlimit fsize;
	fsize.rlim_cur = newlimit * 512;
	fsize.rlim_max = newlimit * 512;
	
	return setrlimit(RLIMIT_FSIZE, &fsize);
      }
    case 3:
      /* Get maximum address for `brk'.  */
      {
	struct rlimit dsize;

	status = getrlimit(RLIMIT_DATA, &dsize);
	if (status < 0)
	  return -1;

	return ((long int) &_etext) + dsize.rlim_cur;
      }
    case 4:
      return sysconf(_SC_OPEN_MAX);

    default:
      errno = EINVAL;
      return -1;
    }
}
