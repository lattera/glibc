/* Copyright (C) 2005
   Free Software Foundation, Inc.
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

#include <errno.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/utsname.h>

/* The kernel's struct utsname is two bytes larger than a userland struct
   utsname due to the APCS structure size boundary.  */

int
__uname (struct utsname *__name)
{
  char buf[sizeof (struct utsname) + 2];
  int result = INLINE_SYSCALL (uname, 1, buf);

  if (result == 0)
    memcpy (__name, buf, sizeof (struct utsname));

  return result;
}

libc_hidden_def (__uname)
strong_alias (__uname, uname)
libc_hidden_weak (uname)
