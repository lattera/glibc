/* Copyright (C) 1994, 1995 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/signal.h>
#include <setjmp.h>
#include "thread_state.h"
#include <sysdep.h>		/* For stack growth direction.  */
#include "set-hooks.h"
#include <assert.h>
#include "hurdmalloc.h"		/* XXX */

extern void _hurd_longjmp_thread_state (struct machine_thread_state *,
					jmp_buf env, int value);


/* Things that want to be locked while forking.  */
struct
  {
    size_t n;
    struct mutex *locks[0];
  } _hurd_fork_locks;


/* Things that want to be called before we fork, to prepare the parent for
   task_create, when the new child task will inherit our address space.  */
DEFINE_HOOK (_hurd_fork_prepare_hook, (void));

/* Things that want to be called when we are forking, with the above all
   locked.  They are passed the task port of the child.  The child process
   is all set up except for doing proc_child, and has no threads yet.  */
DEFINE_HOOK (_hurd_fork_setup_hook, (void));

/* Things to be run in the child fork.  */
DEFINE_HOOK (_hurd_fork_child_hook, (void));

/* Things to be run in the parent fork.  */
DEFINE_HOOK (_hurd_fork_parent_hook, (void));


/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
pid_t
__fork (void)
{
  jmp_buf env;
  pid_t pid;
  size_t i;
  error_t err;
  thread_t thread_self = __mach_thread_self ();
  struct hurd_sigstate *volatile ss;
  sigset_t pending;

  void unlockss (void)
    {
      __spin_lock (&ss->lock);
      ss->critical_section = 0;
      pending = ss->pending & ~ss->blocked;
      __spin_unlock (&ss->lock);
      /* XXX Copying mutex into child and calling mutex_unlock lossy.  */
      __mutex_unlock (&_hurd_siglock);
      ss = NULL;		/* Make sure we crash if we use it again.  */
    }

  ss = _hurd_self_sigstate ();
  __spin_lock (&ss->lock);
  ss->critical_section = 1;
  __spin_unlock (&ss->lock);
  __mutex_lock (&_hurd_siglock);

  if (! setjmp (env))
    {
      process_t newproc;
      task_t newtask;
      thread_t thread, sigthread;
      mach_port_urefs_t thread_refs, sigthread_refs;
      struct machine_thread_state state;
      mach_msg_type_number_t statecount;
      mach_port_t *portnames = NULL;
      mach_msg_type_number_t nportnames = 0;
      mach_port_type_t *porttypes = NULL;
      mach_msg_type_number_t nporttypes = 0;
      thread_t *threads = NULL;
      mach_msg_type_number_t nthreads = 0;
      int ports_locked = 0;

      /* Run things that prepare for forking before we create the task.  */
      RUN_HOOK (_hurd_fork_prepare_hook, ());

      /* Lock things that want to be locked before we fork.  */
      for (i = 0; i < _hurd_fork_locks.n; ++i)
	__mutex_lock (_hurd_fork_locks.locks[i]);
      
      newtask = MACH_PORT_NULL;
      thread = sigthread = MACH_PORT_NULL;
      newproc = MACH_PORT_NULL;

      /* Lock all the port cells for the standard ports while we copy the
	 address space.  We want to insert all the send rights into the
	 child with the same names.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_lock (&_hurd_ports[i].lock);
      ports_locked = 1;

      /* Create the child task.  It will inherit a copy of our memory.  */
      if (err = __task_create (__mach_task_self (), 1, &newtask))
	goto lose;

      /* Fetch the names of all ports used in this task.  */
      if (err = __mach_port_names (__mach_task_self (),
				   &portnames, &nportnames,
				   &porttypes, &nporttypes))
	goto lose;
      if (nportnames != nporttypes)
	{
	  err = EGRATUITOUS;
	  goto lose;
	}

      /* Get send rights for all the threads in this task.
	 We want to avoid giving these rights to the child.  */
      if (err = __task_threads (__mach_task_self (), &threads, &nthreads))
	goto lose;

      /* Get the child process's proc server port.  We will insert it into
	 the child with the same name as we use for our own proc server
	 port; and we will need it to set the child's message port.  */
      if (err = __proc_task2proc (_hurd_ports[INIT_PORT_PROC].port,
				  newtask, &newproc))
	goto lose;

      /* Insert all our port rights into the child task.  */
      thread_refs = sigthread_refs = 0;
      for (i = 0; i < nportnames; ++i)
	{
	  if (porttypes[i] & MACH_PORT_TYPE_RECEIVE)
	    {
	      /* This is a receive right.  We want to give the child task
		 its own new receive right under the same name.  */
	      err = __mach_port_allocate_name (newtask,
					       MACH_PORT_RIGHT_RECEIVE,
					       portnames[i]);
	      if (err == KERN_NAME_EXISTS)
		{
		  /* It already has a right under this name (?!).  Well,
		     there is this bizarre old Mach IPC feature (in #ifdef
		     MACH_IPC_COMPAT in the ukernel) which results in new
		     tasks getting a new receive right for task special
		     port number 2.  What else might be going on I'm not
		     sure.  So let's check.  */
#if !MACH_IPC_COMPAT
#define TASK_NOTIFY_PORT 2
#endif
		  assert (({ mach_port_t thisport, notify_port;
			     mach_msg_type_name_t poly;
			     (__task_get_special_port (newtask,
						       TASK_NOTIFY_PORT,
						       &notify_port) == 0 &&
			      __mach_port_extract_right 
			      (newtask,
			       portnames[i],
			       MACH_MSG_TYPE_MAKE_SEND,
			       &thisport, &poly) == 0 &&
			      (thisport == notify_port) &&
			      __mach_port_deallocate (__mach_task_self (),
						      thisport) == 0 &&
			      __mach_port_deallocate (__mach_task_self (),
						      notify_port) == 0);
			   }));
		}
	      else if (err)
		goto lose;
	      if (porttypes[i] & MACH_PORT_TYPE_SEND)
		{
		  /* Give the child as many send rights for its receive
		     right as we have for ours.  */
		  mach_port_urefs_t refs;
		  mach_port_t port;
		  mach_msg_type_name_t poly;
		  if (err = __mach_port_get_refs (__mach_task_self (),
						  portnames[i],
						  MACH_PORT_RIGHT_SEND,
						  &refs))
		    goto lose;
		  if (err = __mach_port_extract_right (newtask,
						       portnames[i],
						       MACH_MSG_TYPE_MAKE_SEND,
						       &port, &poly))
		    goto lose;
		  if (portnames[i] == _hurd_msgport)
		    {
		      /* We just created a receive right for the child's
			 message port and are about to insert send rights
			 for it.  Now, while we happen to have a send right
			 for it, give it to the proc server.  */
		      mach_port_t old;
		      if (err = __proc_setmsgport (newproc, port, &old))
			goto lose;
		      if (old != MACH_PORT_NULL)
			/* XXX what to do here? */
			__mach_port_deallocate (__mach_task_self (), old);
		    }
		  if (err = __mach_port_insert_right (newtask,
						      portnames[i],
						      port,
						      MACH_MSG_TYPE_MOVE_SEND))
		    goto lose;
		  if (refs > 1 &&
		      (err = __mach_port_mod_refs (newtask,
						   portnames[i],
						   MACH_PORT_RIGHT_SEND,
						   refs - 1)))
		    goto lose;
		}
	      if (porttypes[i] & MACH_PORT_TYPE_SEND_ONCE)
		{
		  /* Give the child a send-once right for its receive right,
		     since we have one for ours.  */
		  mach_port_t port;
		  mach_msg_type_name_t poly;
		  if (err = __mach_port_extract_right
		      (newtask,
		       portnames[i],
		       MACH_MSG_TYPE_MAKE_SEND_ONCE,
		       &port, &poly))
		    goto lose;
		  if (err = __mach_port_insert_right
		      (newtask,
		       portnames[i], port,
		       MACH_MSG_TYPE_MOVE_SEND_ONCE))
		    goto lose;
		}
	    }
	  else if (porttypes[i] & MACH_PORT_TYPE_SEND)
	    {
	      /* This is a send right or a dead name.
		 Give the child as many references for it as we have.  */
	      mach_port_urefs_t refs, *record_refs = NULL;
	      mach_port_t insert;
	      if (portnames[i] == newtask)
		/* Skip the name we use for the child's task port.  */
		continue;
	      if (portnames[i] == __mach_task_self ())
		/* For the name we use for our own task port,
		   insert the child's task port instead.  */
		insert = newtask;
	      else if (portnames[i] == _hurd_ports[INIT_PORT_PROC].port)
		{
		  /* Get the proc server port for the new task.  */
		  if (err = __proc_task2proc (portnames[i], newtask, &insert))
		    goto lose;
		}
	      else if (portnames[i] == thread_self)
		{
		  /* For the name we use for our own thread port, we will
		     insert the thread port for the child main user thread
		     after we create it.  */
		  insert = MACH_PORT_NULL;
		  record_refs = &thread_refs;
		  /* Allocate a dead name right for this name as a
                     placeholder, so the kernel will not chose this name
                     for any other new port (it might use it for one of the
                     rights created when a thread is created).  */
		  if (err = __mach_port_allocate_name
		      (newtask, MACH_PORT_RIGHT_DEAD_NAME, portnames[i]))
		    goto lose;
		}
	      else if (portnames[i] == _hurd_msgport_thread)
		/* For the name we use for our signal thread's thread port,
		   we will insert the thread port for the child's signal
		   thread after we create it.  */
		{
		  insert = MACH_PORT_NULL;
		  record_refs = &sigthread_refs;
		  /* Allocate a dead name right as a placeholder.  */
		  if (err = __mach_port_allocate_name
		      (newtask, MACH_PORT_RIGHT_DEAD_NAME, portnames[i]))
		    goto lose;
		}
	      else
		{
		  /* Skip the name we use for any of our own thread ports.  */
		  mach_msg_type_number_t j;
		  for (j = 0; j < nthreads; ++j)
		    if (portnames[i] == threads[j])
		      break;
		  if (j < nthreads)
		    continue;

		  insert = portnames[i];
		}
	      /* Find out how many user references we have for
		 the send right with this name.  */
	      if (err = __mach_port_get_refs (__mach_task_self (),
					      portnames[i],
					      MACH_PORT_RIGHT_SEND,
					      record_refs ?: &refs))
		goto lose;
	      if (insert == MACH_PORT_NULL)
		continue;
	      /* Insert the chosen send right into the child.  */
	      err = __mach_port_insert_right (newtask,
					      portnames[i],
					      insert,
					      MACH_MSG_TYPE_COPY_SEND);
	      if (err == KERN_NAME_EXISTS)
		{
		  /* It already has a send right under this name (?!).
		     Well, it starts out with a send right for its task
		     port, and inherits the bootstrap and exception ports
		     from us.  */
		  mach_port_t childport;
		  mach_msg_type_name_t poly;
		  assert (__mach_port_extract_right (newtask, portnames[i],
						     MACH_MSG_TYPE_COPY_SEND,
						     &childport, &poly) == 0 &&
			  childport == insert &&
			  __mach_port_deallocate (__mach_task_self (),
						  childport) == 0);
		}
	      else if (err)
		goto lose;
	      /* Give the child as many user references as we have.  */
	      if (refs > 1 &&
		  (err = __mach_port_mod_refs (newtask,
					       portnames[i],
					       MACH_PORT_RIGHT_SEND,
					       refs - 1)))
		goto lose;
	    }
	}

      /* Unlock the standard port cells.  The child must unlock its own
	 copies too.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_unlock (&_hurd_ports[i].lock);
      ports_locked = 0;

      /* Unlock the signal state.  The child must unlock its own copy too.  */
      unlockss ();

      /* Create the child main user thread and signal thread.  */
      if ((err = __thread_create (newtask, &thread)) ||
	  (err = __thread_create (newtask, &sigthread)))
	goto lose;

      /* Insert send rights for those threads.  We previously allocated
         dead name rights with the names we want to give the thread ports
         in the child as placeholders.  Now deallocate them so we can use
         the names.  */
      if ((err = __mach_port_deallocate (newtask, thread_self)) ||
	  (err = __mach_port_insert_right (newtask, thread_self,
					   thread, MACH_MSG_TYPE_COPY_SEND)))
	goto lose;
      /* We have one extra user reference created at the beginning of this
	 function, accounted for by mach_port_names (and which will thus be
	 accounted for in the child below).  This extra right gets consumed
	 in the child by the store into _hurd_sigthread in the child fork.  */
      if (thread_refs > 1 &&
	  (err = __mach_port_mod_refs (newtask, thread_self,
				       MACH_PORT_RIGHT_SEND,
				       thread_refs - 1)))
	goto lose;
      if ((_hurd_msgport_thread != MACH_PORT_NULL) /* Let user have none.  */
	  && ((err = __mach_port_deallocate (newtask, _hurd_msgport_thread)) ||
	      (err = __mach_port_insert_right (newtask, _hurd_msgport_thread,
					       sigthread,
					       MACH_MSG_TYPE_COPY_SEND))))
	goto lose;
      if (sigthread_refs > 1 &&
	  (err = __mach_port_mod_refs (newtask, _hurd_msgport_thread,
				       MACH_PORT_RIGHT_SEND,
				       sigthread_refs - 1)))
	goto lose;

      /* This seems like a convenient juncture to copy the proc server's
	 idea of what addresses our argv and envp are found at from the
	 parent into the child.  Since we happen to know that the child
	 shares our memory image, it is we who should do this copying.  */
      {
	vm_address_t argv, envp;
	err = (__USEPORT (PROC, __proc_get_arg_locations (port, &argv, &envp))
	       ?: __proc_set_arg_locations (newproc, argv, envp));
	if (err)
	  goto lose;
      }
	    
      /* Set the child signal thread up to run the msgport server function
	 using the same signal thread stack copied from our address space.
	 We fetch the state before longjmp'ing it so that miscellaneous
	 registers not affected by longjmp (such as i386 segment registers)
	 are in their normal default state.  */
      statecount = MACHINE_THREAD_STATE_COUNT;
      if (err = __thread_get_state (_hurd_msgport_thread,
				    MACHINE_THREAD_STATE_FLAVOR,
				    (natural_t *) &state, &statecount))
	goto lose;
#if STACK_GROWTH_UP
      state.SP = __hurd_sigthread_stack_base;
#else
      state.SP = __hurd_sigthread_stack_end;
#endif      
      MACHINE_THREAD_STATE_SET_PC (&state,
				   (unsigned long int) _hurd_msgport_receive);
      if (err = __thread_set_state (sigthread, MACHINE_THREAD_STATE_FLAVOR,
				    (natural_t *) &state, statecount))
	goto lose;
      /* We do not thread_resume SIGTHREAD here because the child
	 fork needs to do more setup before it can take signals.  */

      /* Set the child user thread up to return 1 from the setjmp above.  */
      _hurd_longjmp_thread_state (&state, env, 1);
      if (err = __thread_set_state (thread, MACHINE_THREAD_STATE_FLAVOR,
				    (natural_t *) &state, statecount))
	goto lose;

      /* Get the PID of the child from the proc server.  We must do this
	 before calling proc_child below, because at that point any
	 authorized POSIX.1 process may kill the child task with SIGKILL.  */
      if (err = __USEPORT (PROC, __proc_task2pid (port, newtask, &pid)))
	goto lose;

      /* Register the child with the proc server.  It is important that
	 this be that last thing we do before starting the child thread
	 running.  Once proc_child has been done for the task, it appears
	 as a POSIX.1 process.  Any errors we get must be detected before
	 this point, and the child must have a message port so it responds
	 to POSIX.1 signals.  */
      if (err = __USEPORT (PROC, __proc_child (port, newtask)))
	goto lose;

      /* This must be the absolutely last thing we do; we can't assume that
	 the child will remain alive for even a moment once we do this.  We
	 ignore errors because we have committed to the fork and are not
	 allowed to return them after the process becomes visible to
	 POSIX.1 (which happened right above when we called proc_child).  */
      (void) __thread_resume (thread);

    lose:
      if (ports_locked)
	for (i = 0; i < _hurd_nports; ++i)
	  __spin_unlock (&_hurd_ports[i].lock);

      if (newtask != MACH_PORT_NULL)
	{
	  if (err)
	    __task_terminate (newtask);
	  __mach_port_deallocate (__mach_task_self (), newtask);
	}
      if (thread != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), thread);
      if (sigthread != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), sigthread);
      if (newproc != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), newproc);
      if (thread_self != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), thread_self);

      if (portnames)
	__vm_deallocate (__mach_task_self (),
			 (vm_address_t) portnames,
			 nportnames * sizeof (*portnames));
      if (porttypes)
	__vm_deallocate (__mach_task_self (),
			 (vm_address_t) porttypes,
			 nporttypes * sizeof (*porttypes));
      if (threads)
	{
	  for (i = 0; i < nthreads; ++i)
	    __mach_port_deallocate (__mach_task_self (), threads[i]);
	  __vm_deallocate (__mach_task_self (),
			   (vm_address_t) threads,
			   nthreads * sizeof (*threads));
	}

      /* Run things that want to run in the parent to restore it to
	 normality.  Usually prepare hooks and parent hooks are
	 symmetrical: the prepare hook arrests state in some way for the
	 fork, and the parent hook restores the state for the parent to
	 continue executing normally.  */
      RUN_HOOK (_hurd_fork_parent_hook, ());
    }
  else
    {
      struct hurd_sigstate *oldstates;

      /* We are the child task.  Unlock the standard port cells, which were
         locked in the parent when we copied its memory.  The parent has
         inserted send rights with the names that were in the cells then.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_unlock (&_hurd_ports[i].lock);

      /* We are the only thread in this new task, so we will
	 take the task-global signals.  */
      _hurd_sigthread = thread_self;

      /* Unchain the sigstate structures for threads that existed in the
	 parent task but don't exist in this task (the child process).
	 Delay freeing them until later because some of the further setup
	 and unlocking might be required for free to work.  */
      oldstates = _hurd_sigstates;
      if (oldstates == ss)
	oldstates = ss->next;
      else
	{
	  while (_hurd_sigstates->next != ss)
	    _hurd_sigstates = _hurd_sigstates->next;
	  _hurd_sigstates->next = ss->next;
	}
      ss->next = NULL;
      _hurd_sigstates = ss;

      /* Unlock our copies of the signal state locks.  */
      unlockss ();

      /* Fetch our new process IDs from the proc server.  No need to
	 refetch our pgrp; it is always inherited from the parent (so
	 _hurd_pgrp is already correct), and the proc server will send us a
	 proc_newids notification when it changes.  */
      err = __USEPORT (PROC, __proc_getpids (port, &_hurd_pid, &_hurd_ppid,
					     &_hurd_orphaned));

      /* Run things that want to run in the child task to set up.  */
      RUN_HOOK (_hurd_fork_child_hook, ());

      /* Set up proc server-assisted fault recovery for the signal thread.  */
      _hurdsig_fault_init ();

      /* Start the signal thread listening on the message port.  */
      if (!err)
	err = __thread_resume (_hurd_msgport_thread);

      /* Free the old sigstate structures.  */
      while (oldstates != NULL)
	{
	  struct hurd_sigstate *next = oldstates->next;
	  free (oldstates);
	  oldstates = next;
	}
      /* XXX what to do if we have any errors here? */

      pid = 0;
    }

  /* Unlock things we locked before creating the child task.
     They are locked in both the parent and child tasks.  */
  for (i = 0; i < _hurd_fork_locks.n; ++i)
    __mutex_unlock (_hurd_fork_locks.locks[i]);

  if (pending)
    __msg_sig_post (_hurd_msgport, 0, __mach_task_self ());

  return err ? __hurd_fail (err) : pid;
}

weak_alias (__fork, fork)
