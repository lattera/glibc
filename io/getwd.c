/* Copyright (C) 1991, 1992, 1996 Free Software Foundation, Inc.
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
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* Put the absolute pathname of the current working direction in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
char *
getwd (buf)
     char *buf;
{
#ifndef PATH_MAX
#define PATH_MAX 1024
  char fetchbuf[PATH_MAX];
#else
#define fetchbuf buf
#endif

  if (buf == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  if (getcwd (fetchbuf, PATH_MAX) == NULL)
    {
#if defined HAVE_STRERROR_R || defined _LIBC
      __strerror_r (errno, buf, PATH_MAX);
#else
      strncpy (buf, strerror (errno), PATH_MAX);
#endif
      return NULL;
    }

  if (fetchbuf != buf)
    strcpy (buf, fetchbuf);

  return buf;
}
