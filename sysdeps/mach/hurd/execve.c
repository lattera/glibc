/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <fcntl.h>

/* Replace the current process, executing FILE_NAME with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
int
__execve (file_name, argv, envp)
     const char *file_name;
     char *const argv[];
     char *const envp[];
{
  error_t err;
  file_t file = __file_name_lookup (file_name, O_EXEC, 0);

  if (file == MACH_PORT_NULL)
    return -1;

  /* Hopefully this will not return.  */
  err = _hurd_exec (__mach_task_self (), file, argv, envp);

  /* Oh well.  Might as well be tidy.  */
  __mach_port_deallocate (__mach_task_self (), file);

  return __hurd_fail (err);
}

weak_alias (__execve, execve)
