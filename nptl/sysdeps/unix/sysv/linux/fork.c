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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sysdep.h>
#include <libio/libioP.h>
#include <tls.h>
#include "fork.h"
#include <bits/stdio-lock.h>
#include <assert.h>


unsigned long int *__fork_generation_pointer;


lll_lock_t __fork_lock = LLL_LOCK_INITIALIZER;
LIST_HEAD (__fork_prepare_list);
LIST_HEAD (__fork_parent_list);
LIST_HEAD (__fork_child_list);


static void
fresetlockfiles (void)
{
  _IO_ITER i;

  for (i = _IO_iter_begin(); i != _IO_iter_end(); i = _IO_iter_next(i))
    _IO_lock_init (*((_IO_lock_t *) _IO_iter_file(i)->_lock));
}


pid_t
__libc_fork (void)
{
  pid_t pid;
  list_t *runp;

  /* Get the lock so that the set of registered handlers is not
     inconsistent or changes beneath us.  */
  lll_lock (__fork_lock);

  /* Run all the registered preparation handlers.  In reverse order.  */
  list_for_each_prev (runp, &__fork_prepare_list)
    {
      struct fork_handler *curp;

      curp = list_entry (runp, struct fork_handler, list);

      curp->handler ();
    }

  _IO_list_lock ();

  pid_t ppid = THREAD_GETMEM (THREAD_SELF, tid);

#ifdef ARCH_FORK
  pid = ARCH_FORK ();
#else
# error "ARCH_FORK must be defined so that the CLONE_SETTID flag is used"
  pid = INLINE_SYSCALL (fork, 0);
#endif


  if (pid == 0)
    {
      assert (THREAD_GETMEM (THREAD_SELF, tid) != ppid);

      if (__fork_generation_pointer != NULL)
	*__fork_generation_pointer += 4;

      /* Reset the file list.  These are recursive mutexes.  */
      fresetlockfiles ();

      /* Reset locks in the I/O code.  */
      _IO_list_resetlock ();

      /* Run the handlers registered for the child.  */
      list_for_each (runp, &__fork_child_list)
	{
	  struct fork_handler *curp;

	  curp = list_entry (runp, struct fork_handler, list);

	  curp->handler ();
	}

      /* Initialize the fork lock.  */
      __fork_lock = (lll_lock_t) LLL_LOCK_INITIALIZER;
    }
  else
    {
      assert (THREAD_GETMEM (THREAD_SELF, tid) == ppid);

      /* We execute this even if the 'fork' call failed.  */
      _IO_list_unlock ();

      /* Run the handlers registered for the parent.  */
      list_for_each (runp, &__fork_parent_list)
	{
	  struct fork_handler *curp;

	  curp = list_entry (runp, struct fork_handler, list);

	  curp->handler ();
	}

      /* Release the for lock.  */
      lll_unlock (__fork_lock);
    }

  return pid;
}
weak_alias (__libc_fork, __fork)
weak_alias (__libc_fork, fork)
