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

#include <hurd.h>
#include <hurd/msg_server.h>
#include <hurd/id.h>
#include <string.h>

int
_hurd_refport_secure_p (mach_port_t ref)
{
  if (ref == __mach_task_self ())
    return 1;
  if (__USEPORT (AUTH, ref == port))
    return 1;
  return 0;
}

kern_return_t
_S_msg_add_auth (mach_port_t me,
		 auth_t addauth)
{
  error_t err;
  auth_t newauth;

  if (err = __USEPORT (AUTH,
		       __auth_makeauth (port,
					&addauth, 1, MACH_MSG_TYPE_MOVE_SEND,
					NULL, 0,
					NULL, 0,
					NULL, 0,
					NULL, 0,
					&newauth)))
    return err;

  err = __setauth (newauth);
  __mach_port_deallocate (__mach_task_self (), newauth);
  if (err)
    return errno;

  return 0;
}

kern_return_t
_S_msg_del_auth (mach_port_t me,
		 task_t task,
		 intarray_t uids, mach_msg_type_number_t nuids,
		 intarray_t gids, mach_msg_type_number_t ngids)
{
  error_t err;
  auth_t newauth;

  if (!_hurd_refport_secure_p (task))
    return EPERM;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_id.lock);
  err = _hurd_check_ids ();

  if (!err)
    {
      size_t i, j;
      size_t nu = _hurd_id.gen.nuids, ng = _hurd_id.gen.ngids;
      uid_t newu[nu];
      gid_t newg[ng];

      memcpy (newu, _hurd_id.gen.uids, nu * sizeof (uid_t));
      memcpy (newg, _hurd_id.gen.gids, ng * sizeof (gid_t));

      for (j = 0; j < nuids; ++j)
	{
	  const uid_t uid = uids[j];
	  for (i = 0; i < nu; ++i)
	    if (newu[i] == uid)
	      /* Move the last uid into this slot, and decrease the
		 number of uids so the last slot is no longer used.  */
	      newu[i] = newu[--nu];
	}
      __vm_deallocate (__mach_task_self (),
		       (vm_address_t) uids, nuids * sizeof (uid_t));

      for (j = 0; j < ngids; ++j)
	{
	  const gid_t gid = gids[j];
	  for (i = 0; i < nu; ++i)
	    if (newu[i] == gid)
	      /* Move the last gid into this slot, and decrease the
		 number of gids so the last slot is no longer used.  */
	      newu[i] = newu[--nu];
	}
      __vm_deallocate (__mach_task_self (),
		       (vm_address_t) gids, ngids * sizeof (gid_t));

      err = __USEPORT (AUTH, __auth_makeauth
		       (port,
			NULL, 0, MACH_MSG_TYPE_COPY_SEND,
			newu, nu,
			_hurd_id.aux.uids, _hurd_id.aux.nuids,
			newg, ng,
			_hurd_id.aux.uids, _hurd_id.aux.ngids,
			&newauth));
    }
  __mutex_unlock (&_hurd_id.lock);
  HURD_CRITICAL_END;

  if (err)
    return err;

  err = __setauth (newauth);
  __mach_port_deallocate (__mach_task_self (), newauth);
  if (err)
    return errno;

  return 0;
}
