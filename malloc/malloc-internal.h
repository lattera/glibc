/* Internal declarations for malloc, for use within libc.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _MALLOC_INTERNAL_H
#define _MALLOC_INTERNAL_H

/* Called in the parent process before a fork.  */
void __malloc_fork_lock_parent (void) internal_function attribute_hidden;

/* Called in the parent process after a fork.  */
void __malloc_fork_unlock_parent (void) internal_function attribute_hidden;

/* Called in the child process after a fork.  */
void __malloc_fork_unlock_child (void) internal_function attribute_hidden;


#endif /* _MALLOC_INTERNAL_H */
