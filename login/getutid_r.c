/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

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

#include <libc-lock.h>
#include <errno.h>
#include <stdlib.h>
#include <utmp.h>

#include "utmp-private.h"


/* We have to use the lock in getutent_r.c.  */
__libc_lock_define (extern, __libc_utmp_lock attribute_hidden)


int
__getutid_r (const struct utmp *id, struct utmp *buffer, struct utmp **result)
{
#if (_HAVE_UT_ID - 0) && (_HAVE_UT_TYPE - 0)
  int retval;

  /* Test whether ID has any of the legal types.  */
  if (id->ut_type != RUN_LVL && id->ut_type != BOOT_TIME
      && id->ut_type != OLD_TIME && id->ut_type != NEW_TIME
      && id->ut_type != INIT_PROCESS && id->ut_type != LOGIN_PROCESS
      && id->ut_type != USER_PROCESS && id->ut_type != DEAD_PROCESS)
    /* No, using '<' and '>' for the test is not possible.  */
    {
      __set_errno (EINVAL);
      *result = NULL;
      return -1;
    }

  __libc_lock_lock (__libc_utmp_lock);

  retval = (*__libc_utmp_jump_table->getutid_r) (id, buffer, result);

  __libc_lock_unlock (__libc_utmp_lock);

  return retval;
#else	/* !_HAVE_UT_ID && !_HAVE_UT_TYPE */
  __set_errno (ENOSYS);
  return -1;
#endif
}
weak_alias (__getutid_r, getutid_r)
