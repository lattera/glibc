/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <errno.h>
#include <stdio.h>


/* Read up to N chars into BUF from COOKIE.
   Return how many chars were read, 0 for EOF or -1 for error.  */
int
DEFUN(__stdio_read, (cookie, buf, n),
      PTR cookie AND register char *buf AND register size_t n)
{
  errno = ENOSYS;
  return -1;
}

/* Write up to N chars from BUF to COOKIE.
   Return how many chars were written or -1 for error.  */
int
DEFUN(__stdio_write, (cookie, buf, n),
      PTR cookie AND register CONST char *buf AND register size_t n)
{
  errno = ENOSYS;
  return -1;
}

/* Move COOKIE's file position *POS bytes, according to WHENCE.
   The new file position is stored in *POS.
   Returns zero if successful, nonzero if not.  */
int
DEFUN(__stdio_seek, (cookie, pos, whence),
      PTR cookie AND fpos_t *pos AND int whence)
{
  errno = ENOSYS;
  return -1;
}

/* Close the file associated with COOKIE.
   Return 0 for success or -1 for failure.  */
int
DEFUN(__stdio_close, (cookie), PTR cookie)
{
  errno = ENOSYS;
  return -1;
}

/* Return the POSIX.1 file descriptor associated with COOKIE,
   or -1 for errors.  If COOKIE does not relate to any POSIX.1 file
   descriptor, this should return -1 with errno set to EOPNOTSUPP.  */
int
DEFUN(__stdio_fileno, (cookie), PTR cookie)
{
  errno = ENOSYS;
  return -1;
}


/* Open FILENAME with the mode in M.
   Store the magic cookie associated with the opened file in *COOKIEPTR.
   Return zero on success and nonzero on failure.  */   
int
DEFUN(__stdio_open, (filename, m, cookieptr),
      CONST char *filename AND __io_mode m AND PTR *cookieptr)
{
  errno = ENOSYS;
  return -1;
}


/* Open FILENAME with the mode in M.  Use the same magic cookie
   already in *COOKIEPTR if possible, closing the old cookie with CLOSEFN.  */
int
DEFUN(__stdio_reopen, (filename, m, cookieptr),
      CONST char *filename AND __io_mode m AND
      PTR *cookieptr AND __io_close_fn closefn)
{
  errno = ENOSYS;
  return -1;
}


#ifdef	 HAVE_GNU_LD

#include <gnu-stabs.h>

stub_warning(__stdio_read);
stub_warning(__stdio_write);
stub_warning(__stdio_seek);
stub_warning(__stdio_close);
stub_warning(__stdio_fileno);
stub_warning(__stdio_open);
stub_warning(__stdio_reopen);

#endif	/* GNU stabs.  */
