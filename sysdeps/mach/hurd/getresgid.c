/* getresgid -- fetch effective group ID, real group ID, and saved-set group ID
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

/* Fetch the effective group ID, real group ID, and saved-set group ID,
   of the calling process.  */
int
__getresgid (gid_t *egid, gid_t *rgid, gid_t *sgid)
{
  error_t err;
  gid_t eff, real, saved;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_id.lock);

  err = _hurd_check_ids ();
  if (!err)
    {
      if (_hurd_id.aux.ngids < 1)
	/* We do not even have a real GID.  */
	err = EGRATUITOUS;
      else
	{
	  real = _hurd_id.aux.gids[0];
	  saved = _hurd_id.aux.ngids < 2 ? real :_hurd_id.aux.gids[1];
	  eff = _hurd_id.gen.ngids < 1 ? real : _hurd_id.gen.gids[0];
	}
    }

  __mutex_unlock (&_hurd_id.lock);
  HURD_CRITICAL_END;

  if (err)
    return __hurd_fail (err);

  *egid = eff;
  *rgid = real;
  *sgid = saved;
  return 0;
}
libc_hidden_def (__getresgid)
weak_alias (__getresgid, getresgid)
