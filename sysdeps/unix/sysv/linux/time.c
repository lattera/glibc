/* Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <time.h>

#include <sysdep.h>

#ifdef __NR_time

time_t
time (t)
     time_t *t;
{
  INTERNAL_SYSCALL_DECL (err);
  time_t res = INTERNAL_SYSCALL (time, err, 1, NULL);
  /* There cannot be any error.  */
  if (t != NULL)
    *t = res;
  return res;
}
libc_hidden_def (time)

#else

# include <sysdeps/unix/time.c>

#endif
