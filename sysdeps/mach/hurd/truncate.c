/* Copyright (C) 1991, 92, 93, 94, 95, 97 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <errno.h>
#include <hurd.h>
#include <fcntl.h>

/* Truncate FILE_NAME to LENGTH bytes.  */
int
truncate (file_name, length)
     char *file_name;
     off_t length;
{
  error_t err;
  file_t file = __file_name_lookup (file_name, O_WRITE, 0);

  if (file == MACH_PORT_NULL)
    return -1;

  err = __file_set_size (file, length);
  __mach_port_deallocate (__mach_task_self (), file);

  if (err)
    return __hurd_fail (err);
  return 0;
}
