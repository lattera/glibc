/* Copyright (C) 1993, 1994, 1997 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <errno.h>

/* Execute the file FD refers to, overlaying the running program image.  */

int
fexecve (int fd, char *const argv[], char *const envp[])
{
  error_t err = HURD_DPORT_USE (fd, _hurd_exec (__mach_task_self (), port,
						argv, envp));
  if (! err)
    err = EGRATUITOUS;
  return __hurd_fail (err);
}
