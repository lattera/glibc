/* Copyright (C) 1991, 92, 93, 94, 95, 97, 2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/fd.h>

/* Duplicate FD to FD2, closing the old FD2 and making FD2 be
   open on the same file as FD is.  Return FD2 or -1.  */
int
__dup2 (fd, fd2)
     int fd;
     int fd2;
{
  struct hurd_fd *d;

  /* Extract the ports and flags from FD.  */
  d = _hurd_fd_get (fd);
  if (d == NULL)
    {
      errno = EBADF;
      return -1;
    }

  HURD_CRITICAL_BEGIN;

  __spin_lock (&d->port.lock);
  if (d->port.port == MACH_PORT_NULL)
    {
      __spin_unlock (&d->port.lock);
      errno = EBADF;
      fd2 = -1;
    }
  else if (fd2 == fd)
    /* FD is valid and FD2 is already the same; just return it.  */
    __spin_unlock (&d->port.lock);
  else
    {
      struct hurd_userlink ulink, ctty_ulink;
      int flags = d->flags;
      io_t ctty = _hurd_port_get (&d->ctty, &ctty_ulink);
      io_t port = _hurd_port_locked_get (&d->port, &ulink); /* Unlocks D.  */

      if (fd2 < 0)
	{
	  errno = EBADF;
	  fd2 = -1;
	}
      else
	{
	  /* Get a hold of the destination descriptor.  */
	  struct hurd_fd *d2;

	  if (fd2 >= _hurd_dtablesize)
	    {
	      /* The table is not large enough to hold the destination
		 descriptor.  Enlarge it as necessary to allocate this
		 descriptor.  */
	      __mutex_unlock (&_hurd_dtable_lock);
	      /* We still hold FD1's lock, but this is safe because
		 _hurd_alloc_fd will only examine the cells starting
		 at FD2.  */
	      d2 = _hurd_alloc_fd (NULL, fd2);
	      if (d2)
		__spin_unlock (&d2->port.lock);
	      __mutex_lock (&_hurd_dtable_lock);
	    }
	  else
	    {
	      d2 = _hurd_dtable[fd2];
	      if (d2 == NULL)
		{
		  /* Must allocate a new one.  We don't initialize the port
		     cells with this call so that if it fails (out of
		     memory), we will not have already added user
		     references for the ports, which we would then have to
		     deallocate.  */
		  d2 = _hurd_dtable[fd2] = _hurd_new_fd (MACH_PORT_NULL,
							 MACH_PORT_NULL);
		}
	    }

	  if (d2 == NULL)
	    {
	      fd2 = -1;
	      if (errno == EINVAL)
		errno = EBADF;	/* POSIX.1-1990 6.2.1.2 ll 54-55.  */
	    }
	  else
	    {
	      /* Give the ports each a user ref for the new descriptor.  */
	      __mach_port_mod_refs (__mach_task_self (), port,
				    MACH_PORT_RIGHT_SEND, 1);
	      if (ctty != MACH_PORT_NULL)
		__mach_port_mod_refs (__mach_task_self (), ctty,
				      MACH_PORT_RIGHT_SEND, 1);

	      /* Install the ports and flags in the new descriptor slot.  */
	      __spin_lock (&d2->port.lock);
	      d2->flags = flags & ~FD_CLOEXEC; /* Dup clears FD_CLOEXEC. */
	      _hurd_port_set (&d2->ctty, ctty);
	      _hurd_port_locked_set (&d2->port, port); /* Unlocks D2.  */
	    }
	}
      __mutex_unlock (&_hurd_dtable_lock);

      _hurd_port_free (&d->port, &ulink, port);
      if (ctty != MACH_PORT_NULL)
	_hurd_port_free (&d->ctty, &ctty_ulink, port);
    }

  HURD_CRITICAL_END;

  return fd2;
}
libc_hidden_def (__dup2)
weak_alias (__dup2, dup2)
