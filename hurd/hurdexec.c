/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/signal.h>

/* Overlay TASK, executing FILE with arguments ARGV and environment ENVP.
   If TASK == mach_task_self (), some ports are dealloc'd by the exec server.
   ARGV and ENVP are terminated by NULL pointers.  */
error_t
_hurd_exec (task_t task, file_t file, 
	    char *const argv[], char *const envp[])
{
  error_t err;
  char *args, *env, *ap;
  size_t argslen, envlen;
  int ints[INIT_INT_MAX];
  mach_port_t ports[_hurd_nports];
  struct hurd_userlink ulink_ports[_hurd_nports];
  file_t *dtable;
  int dtablesize;
  struct hurd_port **dtable_cells;
  struct hurd_userlink *ulink_dtable;
  int i;
  char *const *p;
  struct hurd_sigstate *ss;
  mach_port_t *please_dealloc, *pdp;


  /* Pack the arguments into an array with nulls separating the elements.  */
  argslen = 0;
  if (argv != NULL)
    {
      p = argv;
      while (*p != NULL)
	argslen += strlen (*p++) + 1;
      args = __alloca (argslen);
      ap = args;
      for (p = argv; *p != NULL; ++p)
	ap = __memccpy (ap, *p, '\0', ULONG_MAX);
    }
  else
    args = NULL;

  /* Pack the environment into an array with nulls separating elements.  */
  envlen = 0;
  if (envp != NULL)
    {
      p = envp;
      while (*p != NULL)
	envlen += strlen (*p++) + 1;
      env = __alloca (envlen);
      ap = env;
      for (p = envp; *p != NULL; ++p)
	ap = __memccpy (ap, *p, '\0', ULONG_MAX);
    }
  else
    env = NULL;

  /* Load up the ports to give to the new program.  */
  for (i = 0; i < _hurd_nports; ++i)
    if (i == INIT_PORT_PROC && task != __mach_task_self ())
      {
	/* This is another task, so we need to ask the proc server
	   for the right proc server port for it.  */
	if (err = __USEPORT (PROC, __proc_task2proc (port, task, &ports[i])))
	  {
	    while (--i > 0)
	      _hurd_port_free (&_hurd_ports[i], &ulink_ports[i], ports[i]);
	    return err;
	  }
      }
    else
      ports[i] = _hurd_port_get (&_hurd_ports[i], &ulink_ports[i]);


  /* Load up the ints to give the new program.  */
  for (i = 0; i < INIT_INT_MAX; ++i)
    switch (i)
      {
      case INIT_UMASK:
	ints[i] = _hurd_umask;
	break;

      case INIT_SIGMASK:
      case INIT_SIGIGN:
      case INIT_SIGPENDING:
	/* We will set these all below.  */
	break;

      default:
	ints[i] = 0;
      }

  ss = _hurd_self_sigstate ();
  __spin_lock (&ss->lock);
  ints[INIT_SIGMASK] = ss->blocked;
  ints[INIT_SIGPENDING] = ss->pending;
  ints[INIT_SIGIGN] = 0;
  for (i = 1; i < NSIG; ++i)
    if (ss->actions[i].sa_handler == SIG_IGN)
      ints[INIT_SIGIGN] |= __sigmask (i);

  /* We hold the sigstate lock until the exec has failed so that no signal
     can arrive between when we pack the blocked and ignored signals, and
     when the exec actually happens.  A signal handler could change what
     signals are blocked and ignored.  Either the change will be reflected
     in the exec, or the signal will never be delivered.  Setting the
     critical section flag avoids anything we call trying to acquire the
     sigstate lock.  */
  
  ss->critical_section = 1;
  __spin_unlock (&ss->lock);

  /* Pack up the descriptor table to give the new program.  */
  __mutex_lock (&_hurd_dtable_lock);

  dtablesize = _hurd_dtable ? _hurd_dtablesize : _hurd_init_dtablesize;

  if (task == __mach_task_self ())
    /* Request the exec server to deallocate some ports from us if the exec
       succeeds.  The init ports and descriptor ports will arrive in the
       new program's exec_startup message.  If we failed to deallocate
       them, the new program would have duplicate user references for them.
       But we cannot deallocate them ourselves, because we must still have
       them after a failed exec call.  */
    please_dealloc = __alloca ((_hurd_nports + (2 * dtablesize))
				* sizeof (mach_port_t));
  else
    please_dealloc = NULL;
  pdp = please_dealloc;

  if (_hurd_dtable != NULL)
    {
      dtable = __alloca (dtablesize * sizeof (dtable[0]));
      ulink_dtable = __alloca (dtablesize * sizeof (ulink_dtable[0]));
      dtable_cells = __alloca (dtablesize * sizeof (dtable_cells[0]));
      for (i = 0; i < dtablesize; ++i)
	{
	  struct hurd_fd *const d = _hurd_dtable[i];
	  if (d == NULL)
	    {
	      dtable[i] = MACH_PORT_NULL;
	      continue;
	    }
	  __spin_lock (&d->port.lock);
	  if (d->flags & FD_CLOEXEC)
	    {
	      /* This descriptor is marked to be closed on exec.
		 So don't pass it to the new program.  */
	      dtable[i] = MACH_PORT_NULL;
	      if (pdp && d->port.port != MACH_PORT_NULL)
		{
		  /* We still need to deallocate the ports.  */
		  *pdp++ = d->port.port;
		  if (d->ctty.port != MACH_PORT_NULL)
		    *pdp++ = d->ctty.port;
		}
	      __spin_unlock (&d->port.lock);
	    }
	  else
	    {
	      if (pdp && d->ctty.port != MACH_PORT_NULL)
		/* All the elements of DTABLE are added to PLEASE_DEALLOC
		   below, so we needn't add the port itself.
		   But we must deallocate the ctty port as well as
		   the normal port that got installed in DTABLE[I].  */
		*pdp++ = d->ctty.port;
	      dtable[i] = _hurd_port_locked_get (&d->port, &ulink_dtable[i]);
	      dtable_cells[i] = &d->port;
	    }
	}
    }
  else
    {
      dtable = _hurd_init_dtable;
      ulink_dtable = NULL;
      dtable_cells = NULL;
    }

  /* The information is all set up now.  Try to exec the file.  */

  {
    if (pdp)
      {
	/* Request the exec server to deallocate some ports from us if the exec
	   succeeds.  The init ports and descriptor ports will arrive in the
	   new program's exec_startup message.  If we failed to deallocate
	   them, the new program would have duplicate user references for them.
	   But we cannot deallocate them ourselves, because we must still have
	   them after a failed exec call.  */

	for (i = 0; i < _hurd_nports; ++i)
	  *pdp++ = ports[i];
	for (i = 0; i < dtablesize; ++i)
	  *pdp++ = dtable[i];
      }

    err = __file_exec (file, task,
		       _hurd_exec_flags & EXEC_INHERITED,
		       args, argslen, env, envlen,
		       dtable, MACH_MSG_TYPE_COPY_SEND, dtablesize, 
		       ports, MACH_MSG_TYPE_COPY_SEND, _hurd_nports, 
		       ints, INIT_INT_MAX,
		       please_dealloc, pdp - please_dealloc,
		       NULL, 0);
  }

  /* Release references to the standard ports.  */
  for (i = 0; i < _hurd_nports; ++i)
    if (i == INIT_PORT_PROC && task != __mach_task_self ())
      __mach_port_deallocate (__mach_task_self (), ports[i]);
    else
      _hurd_port_free (&_hurd_ports[i], &ulink_ports[i], ports[i]);

  if (ulink_dtable != NULL)
    /* Release references to the file descriptor ports.  */
    for (i = 0; i < dtablesize; ++i)
      if (dtable[i] != MACH_PORT_NULL)
	_hurd_port_free (dtable_cells[i], &ulink_dtable[i], dtable[i]);

  /* Release lock on the file descriptor table. */
  __mutex_unlock (&_hurd_dtable_lock);

  /* Safe to let signals happen now.  */
  {
    sigset_t pending;
    __spin_lock (&ss->lock);
    ss->critical_section = 0;
    pending = ss->pending & ~ss->blocked;
    __spin_unlock (&ss->lock);
    if (pending)
      __msg_sig_post (_hurd_msgport, 0, __mach_task_self ());
  }

  return err;
}
