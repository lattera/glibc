/* setresgid -- set real group ID, effective group ID, and saved-set group ID
   Copyright (C) 2002, 2005, 2006 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd.h>
#include <hurd/id.h>

/* Set the real group ID, effective group ID, and saved-set group ID,
   of the calling process to RGID, EGID, and SGID, respectively.  */
int
__setresgid (gid_t rgid, gid_t egid, gid_t sgid)
{
  auth_t newauth;
  error_t err;
  gid_t agids[2] = { rgid, sgid };

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_id.lock);
  err = _hurd_check_ids ();

  if (!err)
    {
      /* Make a new auth handle which has EGID as the first element in the
         list of effective gids.  */

      if (_hurd_id.gen.ngids > 0)
	{
	  _hurd_id.gen.gids[0] = egid;
	  _hurd_id.valid = 0;
	}
      if (_hurd_id.aux.ngids > 1)
	{
	  _hurd_id.aux.gids[0] = rgid;
	  _hurd_id.aux.gids[1] = sgid;
	  _hurd_id.valid = 0;
	}

      err = __USEPORT (AUTH, __auth_makeauth
		       (port, NULL, MACH_MSG_TYPE_COPY_SEND, 0,
			_hurd_id.gen.uids, _hurd_id.gen.nuids,
			_hurd_id.aux.uids, _hurd_id.aux.nuids,
			_hurd_id.gen.ngids ? _hurd_id.gen.gids : &egid,
			_hurd_id.gen.ngids ?: 1,
			_hurd_id.aux.ngids > 1 ? _hurd_id.aux.gids : agids,
			_hurd_id.aux.ngids > 1 ? _hurd_id.aux.ngids : 2,
			&newauth));
    }

  __mutex_unlock (&_hurd_id.lock);
  HURD_CRITICAL_END;

  if (err)
    return __hurd_fail (err);

  /* Install the new handle and reauthenticate everything.  */
  err = __setauth (newauth);
  __mach_port_deallocate (__mach_task_self (), newauth);
  return err;
}
libc_hidden_def (__setresgid)
weak_alias (__setresgid, setresgid)
