/* Copyright (C) 1992, 1995 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

/* Put the name of the current host in no more than LEN bytes of NAME.
   The result is null-terminated if LEN is large enough for the full
   name and the terminator.  */
int
__gethostname (name, len)
     char *name;
     size_t len;
{
  struct utsname buf;

  if (name == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  if (uname (&buf))
    return -1;

  if (strlen (buf.nodename) + 1 > len)
    {
      errno = EINVAL;
      return -1;
    }

  strcpy (name, buf.nodename);
  return 0;
}

weak_alias (__gethostname, gethostname)
