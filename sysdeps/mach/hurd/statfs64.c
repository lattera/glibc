/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <sys/statfs.h>

#include "statfsconv.c"

/* Return information about the filesystem on which FILE resides.  */
int
__statfs64 (const char *file, struct statfs64 *buf)
{
  int result;
  struct statfs buf32;

  /* XXX We simply call __statfs and convert the result to `struct
     statfs64'.  We can probably get away with that since we don't
     support large files on the Hurd yet.  */
  result = __statfs (file, &buf32);
  if (result == 0)
    statfs64_conv (&buf32, buf);

  return result;
}

weak_alias (__statfs64, statfs64)
