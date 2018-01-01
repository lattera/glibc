/* Call to terminate the current thread.  Stub version.
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

/* This causes the current thread to exit, without affecting other
   threads in the process if there are any.  If there are no other
   threads left, then this has the effect of _exit (0).  */

static inline void __attribute__ ((noreturn, always_inline, unused))
__exit_thread (void)
{
  while (1)
    asm ("write me!");
}
