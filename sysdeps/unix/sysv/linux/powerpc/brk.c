/* brk system call for Linux/ppc.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>

void *__curbrk;

int
__brk (void *addr)
{
  register void *syscall_arg asm ("r3") = addr;
  register int syscall_number asm ("r0") = SYS_ify (brk);
  register void *newbrk asm ("r3");
  asm ("sc"
       : "=r" (newbrk)
       : "r" (syscall_arg), "r" (syscall_number)
       : "r4","r5","r6","r7","r8","r9","r10","r11","r12",
         "ctr", "mq", "cr0", "cr1", "cr6", "cr7");

  __curbrk = newbrk;

  if (newbrk < addr)
    {
      __set_errno (ENOMEM);
      return -1;
    }

  return 0;
}
weak_alias (__brk, brk)
