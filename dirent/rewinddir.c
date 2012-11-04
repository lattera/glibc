/* Copyright (C) 1991, 1995, 1996, 1997, 2011 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <dirent.h>


/* Rewind DIRP to the beginning of the directory.  */
void
rewinddir (dirp)
     DIR *dirp;
{
  __set_errno (ENOSYS);
  /* No way to indicate failure.	*/
}
libc_hidden_def (rewinddir)


stub_warning (rewinddir)
