/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <string.h>
#include "linux-dirent.h"

#ifndef DT_UNKNOWN
# define DT_UNKNOWN 0
#endif


void
__dirent_aix_to_linux (const struct aixdirent *aixdir,
		       struct dirent *linuxdir)
{
  linuxdir->d_ino = aixdir->d_ino;
  linuxdir->d_off = aixdir->d_off;
  linuxdir->d_reclen = aixdir->d_reclen;
  linuxdir->d_type = DT_UNKNOWN;
  memcpy (linuxdir->d_name, aixdir->d_name, aixdir->d_namlen + 1);
}


void
__dirent64_aix_to_linux (const struct aixdirent64 *aixdir,
			 struct dirent64 *linuxdir)
{
  linuxdir->d_ino = aixdir->d_ino;
  linuxdir->d_off = aixdir->d_off;
  linuxdir->d_reclen = aixdir->d_reclen;
  linuxdir->d_type = DT_UNKNOWN;
  memcpy (linuxdir->d_name, aixdir->d_name, aixdir->d_namlen + 1);
}
