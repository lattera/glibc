/* Copyright (C) 2000, 2007, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

/* Generate a unique temporary file name from TEMPLATE.  The last six
   characters before a suffix of length SUFFIXLEN of TEMPLATE must be
   "XXXXXX"; they are replaced with a string that makes the filename
   unique.  Then open the file and return a fd. */
int
mkostemps64 (template, suffixlen, flags)
     char *template;
     int suffixlen;
     int flags;
{
  if (suffixlen < 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __gen_tempname (template, suffixlen, flags | O_LARGEFILE, __GT_FILE);
}
