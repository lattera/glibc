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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

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
  INLINE_SYSCALL (clone, 3, 0, CLONE_PARENT_SETTID | SIGCHLD, &pid)
#endif

#include "../system.c"
