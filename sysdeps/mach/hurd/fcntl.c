/* Copyright (C) 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <errno.h>
#include <fcntl.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdarg.h>


/* Perform file control operations on FD.  */
int
DEFUN(__fcntl, (fd, cmd), int fd AND int cmd DOTS)
{
  va_list ap;
  struct hurd_fd *d;
  int result;

  d = _hurd_fd_get (fd);

  if (d == NULL)
    return __hurd_fail (EBADF);

  va_start (ap, cmd);

  switch (cmd)
    {
      error_t err;

    default:			/* Bad command.  */
      errno = EINVAL;
      result = -1;
      break;

      /* First the descriptor-based commands, which do no RPCs.  */

    case F_DUPFD:		/* Duplicate the file descriptor.  */
      {
	struct hurd_fd *new;
	io_t port, ctty;
	struct hurd_userlink ulink, ctty_ulink;
	int flags;

	HURD_CRITICAL_BEGIN;

	/* Extract the ports and flags from the file descriptor.  */
	__spin_lock (&d->port.lock);
	flags = d->flags;
	ctty = _hurd_port_get (&d->ctty, &ctty_ulink);
	port = _hurd_port_locked_get (&d->port, &ulink); /* Unlocks D.  */

	/* Get a new file descriptor.  The third argument to __fcntl is the
	   minimum file descriptor number for it.  */
	new = _hurd_alloc_fd (&result, va_arg (ap, int));
	if (new == NULL)
	  /* _hurd_alloc_fd has set errno.  */
	  result = -1;
	else
	  {
	    /* Give the ports each a user ref for the new descriptor.  */
	    __mach_port_mod_refs (__mach_task_self (), port,
				  MACH_PORT_RIGHT_SEND, 1);
	    if (ctty != MACH_PORT_NULL)
	      __mach_port_mod_refs (__mach_task_self (), ctty,
				    MACH_PORT_RIGHT_SEND, 1);

	    /* Install the ports and flags in the new descriptor.  */
	    if (ctty != MACH_PORT_NULL)
	      _hurd_port_set (&new->ctty, ctty);
	    /* Duplication clears the FD_CLOEXEC flag.  */
	    new->flags = flags & ~FD_CLOEXEC;
	    _hurd_port_locked_set (&new->port, port); /* Unlocks NEW.  */
	  }

	HURD_CRITICAL_END;

	_hurd_port_free (&d->port, &ulink, port);
	if (ctty != MACH_PORT_NULL)
	  _hurd_port_free (&d->ctty, &ctty_ulink, port);

	break;
      }

      /* Set RESULT by evaluating EXPR with the descriptor locked.
	 Check for an empty descriptor and return EBADF.  */
#define LOCKED(expr)							      \
      HURD_CRITICAL_BEGIN;						      \
      __spin_lock (&d->port.lock);					      \
      if (d->port.port == MACH_PORT_NULL)				      \
	result = __hurd_fail (EBADF);					      \
      else								      \
	result = (expr);						      \
      __spin_unlock (&d->port.lock);					      \
      HURD_CRITICAL_END;

    case F_GETFD:		/* Get descriptor flags.  */
      LOCKED (d->flags);
      break;

    case F_SETFD:		/* Set descriptor flags.  */
      LOCKED ((d->flags = va_arg (ap, int), 0));
      break;


      /* Now the real io operations, done by RPCs to io servers.  */

    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      {
	struct flock *fl = va_arg (ap, struct flock *);
	errno = fl?ENOSYS:EINVAL; /* XXX mib needs to implement io rpcs.  */
	result = -1;
	break;
      }

    case F_GETFL:		/* Get per-open flags.  */
      if (err = HURD_FD_PORT_USE (d, __io_get_openmodes (port, &result)))
	result = __hurd_dfail (fd, err);
      break;

    case F_SETFL:		/* Set per-open flags.  */
      err = HURD_FD_PORT_USE (d, __io_set_all_openmodes (port,
							 va_arg (ap, int)));
      result = err ? __hurd_dfail (fd, err) : 0;

    case F_GETOWN:		/* Get owner.  */
      if (err = HURD_FD_PORT_USE (d, __io_get_owner (port, &result)))
	result = __hurd_dfail (fd, err);
      break;

    case F_SETOWN:		/* Set owner.  */
      err = HURD_FD_PORT_USE (d, __io_mod_owner (port, va_arg (ap, pid_t)));
      result = err ? __hurd_dfail (fd, err) : 0;
      break;
    }

  va_end (ap);

  return result;
}

weak_alias (__fcntl, fcntl)
