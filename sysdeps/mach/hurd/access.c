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

#include <ansidecl.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/port.h>
#include <hurd/id.h>
#include <fcntl.h>

/* Test for access to FILE by our real user and group IDs.  */
int
DEFUN(__access, (file, type), CONST char *file AND int type)
{
  error_t err;
  file_t crdir, cwdir, rcrdir, rcwdir, io;
  struct hurd_userlink crdir_ulink, cwdir_ulink;
  int flags, allowed;
  mach_port_t ref;

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
					    _hurd_id.aux.gids, 1,
					    _hurd_id.aux.uids,
					    _hurd_id.aux.nuids,
					    _hurd_id.aux.gids,
					    _hurd_id.aux.ngids,
					    &_hurd_id.rid_auth)))
	goto lose;
    }

  /* Get a port to our root directory, authenticated with the real IDs.  */
  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink);
  ref = __mach_reply_port ();
  err = __io_reauthenticate (crdir, ref, MACH_MSG_TYPE_MAKE_SEND);
  if (!err)
    err = __auth_user_authenticate (_hurd_id.rid_auth,
				    crdir,
				    ref, MACH_MSG_TYPE_MAKE_SEND,
				    &rcrdir);
  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink, crdir);
  __mach_port_destroy (__mach_task_self (), ref);

  if (!err)
    {
      /* Get a port to our current working directory, authenticated with
         the real IDs.  */
      cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &cwdir_ulink);
      ref = __mach_reply_port ();
      err = __io_reauthenticate (cwdir, ref, MACH_MSG_TYPE_MAKE_SEND);
      if (!err)
	err = __auth_user_authenticate (_hurd_id.rid_auth,
					cwdir,
					ref, MACH_MSG_TYPE_MAKE_SEND,
					&rcwdir);
      _hurd_port_free (&_hurd_ports[INIT_PORT_CWDIR], &cwdir_ulink, cwdir);
      __mach_port_destroy (__mach_task_self (), ref);
    }

  /* We are done with _hurd_id.rid_auth now.  */
 lose:
  __mutex_unlock (&_hurd_id.lock);

  HURD_CRITICAL_END;

  if (err)
    return __hurd_fail (err);

  /* Now do a path lookup on FILE, using the crdir and cwdir
     reauthenticated with _hurd_id.rid_auth.  */

  err = __hurd_file_name_lookup (rcrdir, rcwdir, file, 0, 0, &io);
  __mach_port_deallocate (__mach_task_self (), rcrdir);
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
