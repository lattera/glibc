/* setresuid -- set effective user ID, real user ID, and saved-set user ID
   Copyright (C) 2002, 2005 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd.h>
#include <hurd/id.h>

/* Set the effective user ID, real user ID, and saved-set user ID,
   of the calling process to EUID, RUID, and SUID, respectively.  */
int
__setresuid (uid_t euid, uid_t ruid, uid_t suid)
{
  auth_t newauth;
  error_t err;
  uid_t auids[2] = { ruid, suid };

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_id.lock);
  err = _hurd_check_ids ();

  if (!err)
    {
      /* Make a new auth handle which has EUID as the first element in the
         list of effective uids.  */

      if (_hurd_id.gen.nuids > 0)
	{
	  _hurd_id.gen.uids[0] = euid;
	  _hurd_id.valid = 0;
	}
      if (_hurd_id.aux.nuids > 1)
	{
	  _hurd_id.aux.uids[0] = ruid;
	  _hurd_id.aux.uids[1] = suid;
	  _hurd_id.valid = 0;
	}

      err = __USEPORT (AUTH, __auth_makeauth
		       (port, NULL, MACH_MSG_TYPE_COPY_SEND, 0,
			_hurd_id.gen.nuids ? _hurd_id.gen.uids : &euid,
			_hurd_id.gen.nuids ?: 1,
			_hurd_id.aux.nuids > 1 ? _hurd_id.aux.uids : auids,
			_hurd_id.aux.nuids > 1 ? _hurd_id.aux.nuids : 2,
			_hurd_id.gen.gids, _hurd_id.gen.ngids,
			_hurd_id.aux.gids, _hurd_id.aux.ngids,
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
libc_hidden_def (__setresuid)
weak_alias (__setresuid, setresuid)
