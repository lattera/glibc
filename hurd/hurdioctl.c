/* ioctl commands which must be done in the C library.
Copyright (C) 1994, 1995, 1996 Free Software Foundation, Inc.
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
#include <sys/ioctl.h>
#include <hurd/ioctl.h>



/* Symbol set of ioctl handler lists.  If there are user-registered
   handlers, one of these lists will contain them.  The other lists are
   handlers built into the library.  */
symbol_set_define (_hurd_ioctl_handler_lists)

/* Look up REQUEST in the set of handlers.  */
ioctl_handler_t
_hurd_lookup_ioctl_handler (int request)
{
  void *const *ptr;
  const struct ioctl_handler *h;

  /* Mask off the type bits, so that we see requests in a single group as a
     contiguous block of values.  */
  request = _IOC_NOTYPE (request);

  for (ptr = symbol_set_first_element (_hurd_ioctl_handler_lists);
       !symbol_set_end_p (_hurd_ioctl_handler_lists, ptr);
       ++ptr)
    for (h = *ptr; h != NULL; h = h->next)
      if (request >= h->first_request && request <= h->last_request)
	return h->handler;

  return NULL;
}

#include <fcntl.h>

/* Find out how many bytes may be read from FD without blocking.  */

static int
fioctl (int fd,
	int request,
	int *arg)
{
  error_t err;

  *(volatile int *) arg = *arg;

  switch (request)
    {
    default:
      err = ENOTTY;
      break;

    case FIONREAD:
      {
	mach_msg_type_number_t navail;
	err = HURD_DPORT_USE (fd, __io_readable (port, &navail));
	if (!err)
	  *arg = (int) navail;
      }
      break;

    case FIONBIO:
      err = HURD_DPORT_USE (fd, (*arg ?
				 __io_set_some_openmodes :
				 __io_clear_some_openmodes)
			    (port, O_NONBLOCK));
      break;

    case FIOASYNC:
      err = HURD_DPORT_USE (fd, (*arg ?
				 __io_set_some_openmodes :
				 __io_clear_some_openmodes)
			    (port, O_ASYNC));
      break;

    case FIOSETOWN:
      err = HURD_DPORT_USE (fd, __io_mod_owner (port, *arg));
      break;

    case FIOGETOWN:
      err = HURD_DPORT_USE (fd, __io_get_owner (port, arg));
      break;
    }

  return err ? __hurd_dfail (fd, err) : 0;
}

_HURD_HANDLE_IOCTLS (fioctl, FIOGETOWN, FIONREAD);


static int
fioclex (int fd,
	 int request)
{
  int flag;

  switch (request)
    {
    default:
      return __hurd_fail (ENOTTY);
    case FIOCLEX:
      flag = FD_CLOEXEC;
      break;
    case FIONCLEX:
      flag = 0;
      break;
    }

  return __fcntl (fd, F_SETFD, flag);
}
_HURD_HANDLE_IOCTL (fioclex, FIOCLEX, FIONCLEX);

#include <hurd/term.h>
#include <hurd/tioctl.h>

static void
rectty_dtable (mach_port_t cttyid)
{
  int i;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  for (i = 0; i < _hurd_dtablesize; ++i)
    {
      struct hurd_fd *const d = _hurd_dtable[i];
      mach_port_t newctty;

      if (d == NULL)
	/* Nothing to do for an unused descriptor cell.  */
	continue;

      if (cttyid == MACH_PORT_NULL)
	/* We now have no controlling tty at all.  */
	newctty = MACH_PORT_NULL;
      else
	HURD_PORT_USE (&d->port,
		       ({ mach_port_t id;
			  /* Get the io object's cttyid port.  */
			  if (! __term_getctty (port, &id))
			    {
			      if (id == cttyid && /* Is it ours?  */
				  /* Get the ctty io port.  */
				  __term_open_ctty (port,
						    _hurd_pid, _hurd_pgrp,
						    &newctty))
				/* XXX it is our ctty but the call failed? */
				newctty = MACH_PORT_NULL;
			      __mach_port_deallocate
				(__mach_task_self (), (mach_port_t) id);
			    }
			  else
			    newctty = MACH_PORT_NULL;
			  0;
			}));

      /* Install the new ctty port.  */
      _hurd_port_set (&d->ctty, newctty);
    }

  __mutex_unlock (&_hurd_dtable_lock);
  HURD_CRITICAL_END;
}


/* Called when we have received a message saying to use a new ctty ID port.  */

error_t
_hurd_setcttyid (mach_port_t cttyid)
{
  error_t err;

  if (cttyid != MACH_PORT_NULL)
    {
      /* Give the new send right a user reference.
	 This is a good way to check that it is valid.  */
      if (err = __mach_port_mod_refs (__mach_task_self (), cttyid,
				      MACH_PORT_RIGHT_SEND, 1))
	return err;
    }

  /* Install the port, consuming the reference we just created.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], cttyid);

  /* Reset all the ctty ports in all the descriptors.  */
  __USEPORT (CTTYID, (rectty_dtable (cttyid), 0));

  return 0;
}


/* Make FD be the controlling terminal.
   This function is called for `ioctl (fd, TCIOSCTTY)'.  */

static int
tiocsctty (int fd,
	   int request)		/* Always TIOCSCTTY.  */
{
  mach_port_t cttyid;
  error_t err;

  /* Get FD's cttyid port, unless it is already ours.  */
  err = HURD_DPORT_USE (fd, ctty != MACH_PORT_NULL ? EADDRINUSE :
			__term_getctty (port, &cttyid));
  if (err == EADDRINUSE)
    /* FD is already the ctty.  Nothing to do.  */
    return 0;
  else if (err)
    return __hurd_fail (err);

  /* Change the terminal's pgrp to ours.  */
  err = HURD_DPORT_USE (fd, __tioctl_tiocspgrp (port, _hurd_pgrp));
  if (err)
    return __hurd_fail (err);

  /* Make it our own.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], cttyid);

  /* Reset all the ctty ports in all the descriptors.  */
  __USEPORT (CTTYID, (rectty_dtable (cttyid), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocsctty, TIOCSCTTY);

/* Dissociate from the controlling terminal.  */

static int
tiocnotty (int fd,
	   int request)		/* Always TIOCNOTTY.  */
{
  mach_port_t fd_cttyid;
  error_t err;

  if (err = HURD_DPORT_USE (fd, __term_getctty (port, &fd_cttyid)))
    return __hurd_fail (err);

  if (__USEPORT (CTTYID, port != fd_cttyid))
    err = EINVAL;

  __mach_port_deallocate (__mach_task_self (), fd_cttyid);

  if (err)
    return __hurd_fail (err);

  /* Clear our cttyid port cell.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], MACH_PORT_NULL);

  /* Reset all the ctty ports in all the descriptors.  */

  __USEPORT (CTTYID, (rectty_dtable (MACH_PORT_NULL), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocnotty, TIOCNOTTY);
