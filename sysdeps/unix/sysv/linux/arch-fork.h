/* ARCH_FORK definition for Linux fork implementation.  Stub version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

/* This file should define the function-like macro of no arguments
   ARCH_FORK to an INLINE_SYSCALL invocation of the clone-like system
   call, passing the CLONE_CHILD_SETTID and CLONE_CHILD_CLEARTID flags
   and &THREAD_SELF->tid as the TID address.

   Machines that lack an arch-fork.h header file will hit an #error in
   fork.c; this stub file doesn't contain an #error itself mainly for
   the transition period of migrating old machine-specific fork.c files
   to machine-specific arch-fork.h instead.  */
