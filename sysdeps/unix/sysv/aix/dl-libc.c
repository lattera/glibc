/* Handle loading and unloading shared objects for internal libc purposes.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <stdlib.h>
#include <ldsodefs.h>

void *
__libc_dlopen (const char *name)
{
  return _dl_open (name, RTLD_LAZY, NULL);
}

void *
__libc_dlsym (void *map, const char *name)
{
 return _dl_sym (map, name, NULL);
}

int
__libc_dlclose (void *map)
{
  _dl_close (__map);
  return 0;
}
