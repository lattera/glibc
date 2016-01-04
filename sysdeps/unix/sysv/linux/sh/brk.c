/* brk system call for Linux/SH.
   Copyright (C) 1999-2016 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <sysdep.h>

/* This must be initialized data because commons can't have aliases.  */
void *__curbrk = 0;

int
__brk (void *addr)
{
  void *newbrk;
  register long r3 asm ("%r3") = SYS_ify (brk);
  register long r4 asm ("%r4") = (long)addr;

  asm volatile ("trapa #0x11\n\t" SYSCALL_INST_PAD
		: "=z"(newbrk)
		: "r" (r3), "r" (r4));

  __curbrk = newbrk;

  if (newbrk < addr)
    {
      __set_errno (ENOMEM);
      return -1;
    }

  return 0;
}
weak_alias (__brk, brk)
