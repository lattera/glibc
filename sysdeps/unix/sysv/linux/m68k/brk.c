/* brk system call for Linux/m68k.
Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <sysdep.h>

void *__curbrk;

int
__brk (void *addr)
{
  void *newbrk;

  {
    register long d0 __asm__ ("%d0");

    asm ("move%.l %2, %%d1\n"
	 "trap #0"		/* Perform the system call.  */
	 : "=d" (d0)
	 : "0" (SYS_ify (brk)), "g" (addr)
	 : "%d0", "%d1");
    newbrk = (void *) d0;
  }
  __curbrk = newbrk;

  if (newbrk < addr)
    {
      errno = ENOMEM;
      return -1;
    }

  return 0;
}
weak_alias (__brk, brk)

