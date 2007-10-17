/* Copyright (C) 1992-1997,1999,2000,2002,2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <fcntl.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdarg.h>
#include <sys/file.h>		/* XXX for LOCK_* */

/* Perform file control operations on FD.  */
int
__libc_fcntl (int fd, int cmd, ...)
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
    case F_DUPFD_CLOEXEC:
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

	if (cmd == F_DUPFD_CLOEXEC)
	  flags |= FD_CLOEXEC;
	else
	  /* Duplication clears the FD_CLOEXEC flag.  */
	  flags &= ~FD_CLOEXEC;

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
	    new->flags = flags;
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
	/* XXX
	   We need new RPCs to support POSIX.1 fcntl file locking!!
	   For the time being we support the whole-file case only,
	   with all kinds of WRONG WRONG WRONG semantics,
	   by using flock.  This is definitely the Wrong Thing,
	   but it might be better than nothing (?).  */
	struct flock *fl = va_arg (ap, struct flock *);
	va_end (ap);
	switch (cmd)
	  {
	  case F_GETLK:
	    errno = ENOSYS;
	    return -1;
	  case F_SETLK:
	    cmd = LOCK_NB;
	    break;
	  default:
	    cmd = 0;
	    break;
	  }
	switch (fl->l_type)
	  {
	  case F_RDLCK: cmd |= LOCK_SH; break;
	  case F_WRLCK: cmd |= LOCK_EX; break;
	  case F_UNLCK: cmd |= LOCK_UN; break;
	  default:
	    errno = EINVAL;
	    return -1;
	  }
	switch (fl->l_whence)
	  {
	  case SEEK_SET:
	    if (fl->l_start == 0 && fl->l_len == 0) /* Whole file request.  */
	      break;
	    /* It seems to be common for applications to lock the first
	       byte of the file when they are really doing whole-file locking.
	       So, since it's so wrong already, might as well do that too.  */
	    if (fl->l_start == 0 && fl->l_len == 1)
	      break;
	    /* FALLTHROUGH */
	  case SEEK_CUR:
	  case SEEK_END:
	    errno = ENOTSUP;
	    return -1;
	  default:
	    errno = EINVAL;
	    return -1;
	  }

	return __flock (fd, cmd);
      }

    case F_GETFL:		/* Get per-open flags.  */
      if (err = HURD_FD_PORT_USE (d, __io_get_openmodes (port, &result)))
	result = __hurd_dfail (fd, err);
      break;

    case F_SETFL:		/* Set per-open flags.  */
      err = HURD_FD_PORT_USE (d, __io_set_all_openmodes (port,
							 va_arg (ap, int)));
      result = err ? __hurd_dfail (fd, err) : 0;
      break;

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
libc_hidden_def (__libc_fcntl)
weak_alias (__libc_fcntl, __fcntl)
libc_hidden_weak (__fcntl)
weak_alias (__libc_fcntl, fcntl)
