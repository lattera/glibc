/* Copyright (C) 1991, 1992, 1993, 1996, 1997 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define	badmode()	return ((__set_errno (EINVAL)), 0)

/* Dissect the given mode string into an __io_mode.  */
int
__getmode (const char *mode, __io_mode *mptr)
{
  register unsigned char i;

  if (mode == NULL)
    badmode ();

  memset ((void *) mptr, 0, sizeof (*mptr));

  switch (*mode)
    {
    case 'a':
      mptr->__write = mptr->__create = mptr->__append = 1;
      break;
    case 'w':
      mptr->__write = mptr->__create = mptr->__truncate = 1;
      break;
    case 'r':
      mptr->__read = 1;
      break;
    default:
      badmode ();
    }

  for (i = 1; i < 3; ++i)
    {
      ++mode;
      if (*mode == '\0')
	break;
      switch (*mode)
	{
	case '+':
	  mptr->__read = mptr->__write = 1;
	  break;
	case 'b':
	  mptr->__binary = 1;
	  break;
	}
    }

  if (!mptr->__read && !mptr->__write)
    badmode ();

  mptr->__exclusive = *mode == 'x';

  return 1;
}

/* Open a new stream on the given file.  */
FILE *
fopen (filename, mode)
     const char *filename;
     const char *mode;
{
  FILE *stream;
  __io_mode m;

  if (filename == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  if (!__getmode (mode, &m))
    return NULL;

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  if (__stdio_open (filename, m, &stream->__cookie))
    {
      int save = errno;
      (void) fclose (stream);
      __set_errno (save);
      return NULL;
    }

  stream->__mode = m;

  return stream;
}
