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
#include <stdio.h>
#include <hurd.h>

/* Rename the file OLD to NEW.  */
int
DEFUN(rename, (old, new), CONST char *old AND CONST char *new)
{
  error_t err;
  file_t olddir, newdir;
  const char *oldname, *newname;

  olddir = __file_name_split (old, (char **) &oldname);
  if (olddir == MACH_PORT_NULL)
    return -1;
  newdir = __file_name_split (new, (char **) &newname);
  if (newdir == MACH_PORT_NULL)
    {
       __mach_port_deallocate (__mach_task_self (), olddir);
      return -1;
    }

  err = __dir_rename (olddir, oldname, newdir, newname);
  __mach_port_deallocate (__mach_task_self (), olddir);
  __mach_port_deallocate (__mach_task_self (), newdir);
  if (err)
    return __hurd_fail (err);
  return 0;
}
