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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Read N bytes into BUF from COOKIE.  */
int
DEFUN(__stdio_read, (cookie, buf, n),
      PTR cookie AND register char *buf AND register size_t n)
{
  CONST int fd = (int) cookie;
#if	defined (EINTR) && defined (EINTR_REPEAT)
  int save = errno;
  int nread;

 try:;
  errno = 0;
  nread = __read (fd, buf, (int) n);
  if (nread < 0)
    {
      if (errno == EINTR)
	goto try;
      return -1;
    }
  errno = save;
  return nread;

#else	/* No EINTR.  */
  return __read (fd, buf, n);
#endif
}


/* Write N bytes from BUF to COOKIE.  */
int
DEFUN(__stdio_write, (cookie, buf, n),
      PTR cookie AND register CONST char *buf AND register size_t n)
{
  CONST int fd = (int) cookie;
  register size_t written = 0;

  while (n > 0)
    {
      int count = __write (fd, buf, (int) n);
      if (count > 0)
	{
	  buf += count;
	  written += count;
	  n -= count;
	}
      else if (count < 0
#if	defined (EINTR) && defined (EINTR_REPEAT)
	       && errno != EINTR
#endif
	       )
	/* Write error.  */
	return -1;
    }

  return (int) written;
}


/* Move COOKIE's file position *POS bytes, according to WHENCE.
   The new file position is stored in *POS.
   Returns zero if successful, nonzero if not.  */
int
DEFUN(__stdio_seek, (cookie, pos, whence),
      PTR cookie AND fpos_t *pos AND int whence)
{
  off_t new;
  new = __lseek ((int) cookie, (off_t) *pos, whence);
  if (new < 0)
    return 1;
  *pos = (fpos_t) new;
  return 0;
}


/* Close COOKIE.  */
int
DEFUN(__stdio_close, (cookie), PTR cookie)
{
  return __close ((int) cookie);
}

/* Return the POSIX.1 file descriptor associated with COOKIE,
   or -1 for errors.  If COOKIE does not relate to any POSIX.1 file
   descriptor, this should return -1 with errno set to EOPNOTSUPP.  */
int
DEFUN(__stdio_fileno, (cookie), PTR cookie)
{
  return (int) cookie;
}


/* Open the given file with the mode given in the __io_mode argument.  */
int
DEFUN(__stdio_open, (filename, m, cookieptr),
      CONST char *filename AND __io_mode m AND PTR *cookieptr)
{
  int fd;
  int mode;

  if (m.__read && m.__write)
    mode = O_RDWR;
  else
    mode = m.__read ? O_RDONLY : O_WRONLY;

  if (m.__append)
    mode |= O_APPEND;
  if (m.__exclusive)
    mode |= O_EXCL;
  if (m.__truncate)
    mode |= O_TRUNC;

  if (m.__create)
    fd = __open (filename, mode | O_CREAT,
		 S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  else
    fd = __open (filename, mode);

  if (fd < 0)
    return -1;

  *cookieptr = (PTR) fd;
  return 0;
}


/* Open FILENAME with the mode in M.  Use the same magic cookie
   already in *COOKIEPTR if possible, closing the old cookie with CLOSEFN.  */
int
DEFUN(__stdio_reopen, (filename, m, cookieptr),
      CONST char *filename AND __io_mode m AND
      PTR *cookieptr AND __io_close_fn closefn)
{
  PTR newcookie;

  /* We leave the old descriptor open while we open the file.
     That way ``freopen ("/dev/stdin", "r", stdin)'' works.  */

  if (__stdio_open (filename, m, &newcookie))
    {
      if (errno == ENFILE || errno == EMFILE)
	{
	  /* We are out of file descriptors.  Try closing the old one and
	     retrying the open.  */
	  (void) (*closefn) (*cookieptr);
	  if (__stdio_open (filename, m, &newcookie))
	    return -1;
	}
    }

  if (newcookie != *cookieptr)
    {
      if (closefn != __stdio_close ||
	  /* Try to move the descriptor to the desired one.  */
	  __dup2 ((int) newcookie, (int) *cookieptr) < 0)
	/* Didn't work.  Give the caller the new cookie.  */
	*cookieptr = newcookie;
    }

  return 0;
}
