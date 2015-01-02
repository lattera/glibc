/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <hurd.h>
#include <hurd/port.h>
#include <hurd/id.h>
#include <hurd/lookup.h>
#include <fcntl.h>

/* Test for access to FILE by our real user and group IDs.  */
int
__access (file, type)
     const char *file;
     int type;
{
  error_t err;
  file_t rcrdir, rcwdir, io;
  int flags, allowed;

  error_t reauthenticate (int which, file_t *result)
    {
      /* Get a port to our root directory, authenticated with the real IDs.  */
      error_t err;
      mach_port_t ref;
      ref = __mach_reply_port ();
      err = HURD_PORT_USE
	(&_hurd_ports[which],
	 ({
	   err = __io_reauthenticate (port, ref, MACH_MSG_TYPE_MAKE_SEND);
	   if (!err)
	     err = __auth_user_authenticate (_hurd_id.rid_auth,
					     ref, MACH_MSG_TYPE_MAKE_SEND,
					     result);
	   err;
	 }));
      __mach_port_destroy (__mach_task_self (), ref);
      return err;
    }

  error_t init_port (int which, error_t (*operate) (mach_port_t))
    {
      switch (which)
	{
	case INIT_PORT_AUTH:
	  return (*operate) (_hurd_id.rid_auth);
	case INIT_PORT_CRDIR:
	  return (reauthenticate (INIT_PORT_CRDIR, &rcrdir) ?:
		  (*operate) (rcrdir));
	case INIT_PORT_CWDIR:
	  return (reauthenticate (INIT_PORT_CWDIR, &rcwdir) ?:
		  (*operate) (rcwdir));
	default:
	  return _hurd_ports_use (which, operate);
	}
    }

  rcrdir = rcwdir = MACH_PORT_NULL;

  HURD_CRITICAL_BEGIN;

  __mutex_lock (&_hurd_id.lock);
  /* Get _hurd_id up to date.  */
  if (err = _hurd_check_ids ())
    goto lose;

  if (_hurd_id.rid_auth == MACH_PORT_NULL)
    {
      /* Set up _hurd_id.rid_auth.  This is a special auth server port
	 which uses the real uid and gid (the first aux uid and gid) as
	 the only effective uid and gid.  */

      if (_hurd_id.aux.nuids < 1 || _hurd_id.aux.ngids < 1)
	{
	  /* We do not have a real UID and GID.  Lose, lose, lose!  */
	  err = EGRATUITOUS;
	  goto lose;
	}

      /* Create a new auth port using our real UID and GID (the first
	 auxiliary UID and GID) as the only effective IDs.  */
      if (err = __USEPORT (AUTH,
			   __auth_makeauth (port,
					    NULL, MACH_MSG_TYPE_COPY_SEND, 0,
					    _hurd_id.aux.uids, 1,
					    _hurd_id.aux.uids,
					    _hurd_id.aux.nuids,
					    _hurd_id.aux.gids, 1,
					    _hurd_id.aux.gids,
					    _hurd_id.aux.ngids,
					    &_hurd_id.rid_auth)))
	goto lose;
    }

  if (!err)
    /* Look up the file name using the modified init ports.  */
    err = __hurd_file_name_lookup (&init_port, &__getdport, 0,
				   file, 0, 0, &io);

  /* We are done with _hurd_id.rid_auth now.  */
 lose:
  __mutex_unlock (&_hurd_id.lock);

  HURD_CRITICAL_END;

  if (rcrdir != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), rcrdir);
  if (rcwdir != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), rcwdir);
  if (err)
    return __hurd_fail (err);

  /* Find out what types of access we are allowed to this file.  */
  err = __file_check_access (io, &allowed);
  __mach_port_deallocate (__mach_task_self (), io);
  if (err)
    return __hurd_fail (err);

  flags = 0;
  if (type & R_OK)
    flags |= O_READ;
  if (type & W_OK)
    flags |= O_WRITE;
  if (type & X_OK)
    flags |= O_EXEC;

  if (flags & ~allowed)
    /* We are not allowed all the requested types of access.  */
    return __hurd_fail (EACCES);

  return 0;
}

weak_alias (__access, access)
