/* Copyright (C) 1991 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stdlib.h>
#include "exit.h"


/* Register FUNC to be executed by `exit'.  */
int
DEFUN(atexit, (func), void EXFUN((*func), (NOARGS)))
{
  struct exit_function *new = __new_exitfn();

  if (new == NULL)
    return -1;

  new->flavor = ef_at;
  new->func.at = func;
  return 0;
}


static struct exit_function_list fnlist = { NULL, 0, };
struct exit_function_list *__exit_funcs = &fnlist;

struct exit_function *
DEFUN_VOID(__new_exitfn)
{
  register struct exit_function_list *l;

  for (l = __exit_funcs; l != NULL; l = l->next)
    {
      register size_t i;
      for (i = 0; i < l->idx; ++i)
	if (l->fns[i].flavor == ef_free)
	  return &l->fns[i];
      if (l->idx < sizeof(l->fns) / sizeof(l->fns[0]))
	return &l->fns[l->idx++];
    }

  l = (struct exit_function_list *) malloc(sizeof(struct exit_function_list));
  if (l == NULL)
    return NULL;
  l->next = __exit_funcs;
  __exit_funcs = l;

  l->idx = 1;
  return &l->fns[0];
}
