/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/signal.h>
#include <hurd/term.h>
#include <fcntl.h>

/* Store PORT in file descriptor D, doing appropriate ctty magic.
   FLAGS are as for `open'; only O_IGNORE_CTTY is meaningful.
   D should be locked, and will not be unlocked.  */

void
_hurd_port2fd (struct hurd_fd *d, io_t port, int flags)
{
  io_t ctty;
  mach_port_t cttyid;
  int is_ctty = !(flags & O_IGNORE_CTTY) && ! __term_getctty (port, &cttyid);

  if (is_ctty)
    {
      /* This port is capable of being a controlling tty.
	 Is it ours?  */
      struct hurd_port *const id = &_hurd_ports[INIT_PORT_CTTYID];
      __spin_lock (&id->lock);
      if (id->port == MACH_PORT_NULL)
	/* We have no controlling tty, so make this one it.  */
	_hurd_port_locked_set (id, cttyid);
      else
	{
	  if (cttyid != id->port)
	    /* We have a controlling tty and this is not it.  */
	    is_ctty = 0;
	  /* Either we don't want CTTYID, or ID->port already is it.
	     So we don't need to change ID->port, and we can release
	     the reference to CTTYID.  */
	  __spin_unlock (&id->lock);
	  __mach_port_deallocate (__mach_task_self (), cttyid);
	}
    }

  if (!is_ctty || __term_open_ctty (port, _hurd_pid, _hurd_pgrp, &ctty) != 0)
    /* XXX if IS_CTTY, then this port is our ctty, but we are
       not doing ctty style i/o because term_become_ctty barfed.
       What to do?  */
    /* No ctty magic happening here.  */
    ctty = MACH_PORT_NULL;

  /* Install PORT in the descriptor cell, leaving it locked.  */
  {
    mach_port_t old
      = _hurd_userlink_clear (&d->port.users) ? d->port.port : MACH_PORT_NULL;
    d->port.port = port;
    if (old != MACH_PORT_NULL)
      __mach_port_deallocate (__mach_task_self (), old);
  }

  _hurd_port_set (&d->ctty, ctty);
}
