/* getresuid -- fetch effective user ID, real user ID, and saved-set user ID
   Copyright (C) 2002 Free Software Foundation, Inc.
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

/* Fetch the effective user ID, real user ID, and saved-set user ID,
   of the calling process.  */
int
__getresuid (uid_t *euid, uid_t *ruid, uid_t *suid)
{
  error_t err;
  uid_t eff, real, saved;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_id.lock);

  err = _hurd_check_ids ();
  if (!err)
    {
      if (_hurd_id.aux.nuids < 1)
	/* We do not even have a real UID.  */
	err = EGRATUITOUS;
      else
	{
	  real = _hurd_id.aux.uids[0];
	  saved = _hurd_id.aux.nuids < 2 ? real :_hurd_id.aux.uids[1];
	  eff = _hurd_id.gen.nuids < 1 ? real : _hurd_id.gen.uids[0];
	}
    }

  __mutex_unlock (&_hurd_id.lock);
  HURD_CRITICAL_END;

  if (err)
    return __hurd_fail (err);

  *euid = eff;
  *ruid = real;
  *suid = saved;
  return 0;
}
libc_hidden_def (__getresuid)
weak_alias (__getresuid, getresuid)
