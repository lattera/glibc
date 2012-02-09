/* Copyright (C) 1991, 1996, 2005, 2009 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include "exit.h"
#include <atomic.h>
#include <sysdep.h>

/* Register a function to be called by exit.  */
int
__on_exit (void (*func) (int status, void *arg), void *arg)
{
  struct exit_function *new = __new_exitfn (&__exit_funcs);

  if (new == NULL)
    return -1;

#ifdef PTR_MANGLE
  PTR_MANGLE (func);
#endif
  new->func.on.fn = func;
  new->func.on.arg = arg;
  atomic_write_barrier ();
  new->flavor = ef_on;
  return 0;
}
weak_alias (__on_exit, on_exit)
