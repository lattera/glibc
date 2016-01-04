/* Truncate a file (by name).  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <nacl-interfaces.h>

/* Truncate PATH to LENGTH bytes.  */
int
__truncate (const char *path, off_t length)
{
  return NACL_CALL (__nacl_irt_dev_filename.truncate (path, length), 0);
}
weak_alias (__truncate, truncate)

/* truncate64 is the same as truncate.  */
strong_alias (__truncate, __truncate64)
weak_alias (__truncate64, truncate64)
