/* Copyright (C) 1992, 1995, 1997, 1999 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

extern pid_t __syscall_fork __P ((void));

#ifdef __NR_vfork
extern pid_t __syscall_vfork __P ((void));

/* Use the system call.  If it's not available, fork is close enough.  */

pid_t
__vfork __P ((void))
{
  pid_t retval = INLINE_SYSCALL (vfork, 0);

  if (retval == (pid_t) -1 && errno == ENOSYS)
    retval = INLINE_SYSCALL (fork, 0);

  return retval;
}
#else
pid_t
__vfork __P ((void))
{
  return INLINE_SYSCALL (fork, 0);
}
#endif

weak_alias (__vfork, vfork)
