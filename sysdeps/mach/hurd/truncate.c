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
#include <sys/types.h>
#include <errno.h>
#include <hurd.h>
#include <fcntl.h>

/* Truncate FILE_NAME to LENGTH bytes.  */
int
DEFUN(truncate, (file_name, length),
      CONST char *file_name AND off_t length)
{
  error_t err;
  file_t file = __file_name_lookup (file_name, O_WRITE, 0);

  if (file == MACH_PORT_NULL)
    return -1;

  err = __file_truncate (file, length);
  __mach_port_deallocate (__mach_task_self (), file);

  if (err)
    return __hurd_fail (err);
  return 0;
}
