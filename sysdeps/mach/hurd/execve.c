/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd.h>
#include <fcntl.h>

/* Replace the current process, executing FILE_NAME with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
int
DEFUN(__execve, (file_name, argv, envp),
      CONST char *file_name AND char *CONST argv[] AND char *CONST envp[])
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
