/* Copyright (C) 1991, 1996, 1999 Free Software Foundation, Inc.
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

#include <bits/libc-lock.h>
#include <stdlib.h>
#include "exit.h"


/* Register FUNC to be executed by `exit'.  */
int
atexit (void (*func) (void))
{
  struct exit_function *new = __new_exitfn ();

  if (new == NULL)
    return -1;

  new->flavor = ef_at;
  new->func.at = func;
  return 0;
}


/* We change global data, so we need locking.  */
__libc_lock_define_initialized (static, lock)


static struct exit_function_list initial;
struct exit_function_list *__exit_funcs = &initial;

struct exit_function *
__new_exitfn (void)
{
  struct exit_function_list *l;
  size_t i = 0;

  __libc_lock_lock (lock);

  for (l = __exit_funcs; l != NULL; l = l->next)
    {
      for (i = 0; i < l->idx; ++i)
	if (l->fns[i].flavor == ef_free)
	  break;
      if (i < l->idx)
	break;

      if (l->idx < sizeof (l->fns) / sizeof (l->fns[0]))
	{
	  i = l->idx++;
	  break;
	}
    }

  if (l == NULL)
    {
      l = (struct exit_function_list *)
	malloc (sizeof (struct exit_function_list));
      if (l != NULL)
	{
	  l->next = __exit_funcs;
	  __exit_funcs = l;

	  l->idx = 1;
      	  i = 0;
	}
    }

  /* Mark entry as used, but we don't know the flavor now.  */
  if (l != NULL)
    l->fns[i].flavor = ef_us;

  __libc_lock_unlock (lock);

  return l == NULL ? NULL : &l->fns[i];
}
