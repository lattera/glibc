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
#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/port.h>
#include <string.h>

/* Put the name of the current host in no more than LEN bytes of NAME.
   The result is null-terminated if LEN is large enough for the full
   name and the terminator.  */
int
DEFUN(__gethostname, (name, len),
      char *name AND size_t len)
{
  error_t err;
  char *buf = name;
  mach_msg_type_number_t buflen = len;
  if (err = __USEPORT (PROC, __proc_gethostname (port, &buf, &buflen)))
    return __hurd_fail (err);
  if (buf != name)
    {
      memcpy (name, buf, len < buflen ? len : buflen);
      __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
    }
  if (buflen > len)
    return __hurd_fail (ENAMETOOLONG);
  return 0;
}

weak_alias (__gethostname, gethostname)
