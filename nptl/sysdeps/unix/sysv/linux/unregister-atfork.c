/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <stdlib.h>
#include "fork.h"


/* Defined in libc_pthread_init.c.  */
extern struct fork_handler __pthread_child_handler attribute_hidden;
/* Three static memory blocks used when registering malloc.  */
static struct fork_handler malloc_prepare;
static struct fork_handler malloc_parent;
static struct fork_handler malloc_child;


void
__unregister_atfork (dso_handle)
     void *dso_handle;
{
  /* Get the lock to not conflict with running forks.  */
  lll_lock (__fork_lock);

  list_t *runp;
  list_t *prevp;

  list_for_each_prev_safe (runp, prevp, &__fork_prepare_list)
    if (list_entry (runp, struct fork_handler, list)->dso_handle == dso_handle)
      {
	list_del (runp);

	struct fork_handler *p = list_entry (runp, struct fork_handler, list);
	if (p != &malloc_prepare)
	  free (p);
      }

  list_for_each_prev_safe (runp, prevp, &__fork_parent_list)
    if (list_entry (runp, struct fork_handler, list)->dso_handle == dso_handle)
      {
	list_del (runp);

	struct fork_handler *p = list_entry (runp, struct fork_handler, list);
	if (p != &malloc_parent)
	  free (p);
      }

  list_for_each_prev_safe (runp, prevp, &__fork_child_list)
    if (list_entry (runp, struct fork_handler, list)->dso_handle == dso_handle)
      {
	list_del (runp);

	struct fork_handler *p = list_entry (runp, struct fork_handler, list);
	if (p != &__pthread_child_handler && p != &malloc_child)
	  free (p);
      }

  /* Release the lock.  */
  lll_unlock (__fork_lock);
}
