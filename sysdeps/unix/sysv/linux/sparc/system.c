/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <kernel-features.h>

/* We have to and actually can handle cancelable system().  The big
   problem: we have to kill the child process if necessary.  To do
   this a cleanup handler has to be registered and is has to be able
   to find the PID of the child.  The main problem is to reliable have
   the PID when needed.  It is not necessary for the parent thread to
   return.  It might still be in the kernel when the cancellation
   request comes.  Therefore we have to use the clone() calls ability
   to have the kernel write the PID into the user-level variable.  */
#ifdef __ASSUME_CLONE_THREAD_FLAGS
# define FORK() \
  INLINE_CLONE_SYSCALL (CLONE_PARENT_SETTID | SIGCHLD, 0, &pid, NULL, NULL)
#endif

#include "../system.c"
