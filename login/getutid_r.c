/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
int
__getutid_r (const struct utmp *id, struct utmp **utmp,
	     struct utmp_data *utmp_data)
{
#if (_HAVE_UT_ID - 0) && (_HAVE_UT_TYPE - 0)
  /* Test whether ID has any of the legal types.  */
  if (id->ut_type != RUN_LVL && id->ut_type != BOOT_TIME
      && id->ut_type != OLD_TIME && id->ut_type != NEW_TIME
      && id->ut_type != INIT_PROCESS && id->ut_type != LOGIN_PROCESS
      && id->ut_type != USER_PROCESS && id->ut_type != DEAD_PROCESS)
    /* No, using '<' and '>' for the test is not possible.  */
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Open utmp file if not already done.  */
  if (utmp_data->ut_fd == -1)
    {
      setutent_r (utmp_data);
      if (utmp_data->ut_fd == -1)
	return -1;
    }

  /* Position file correctly.  */
  if (lseek (utmp_data->ut_fd, utmp_data->loc_utmp, SEEK_SET) == -1)
    return -1;

  if (id->ut_type == RUN_LVL || id->ut_type == BOOT_TIME
      || id->ut_type == OLD_TIME || id->ut_type == NEW_TIME)
    {
      /* Search for next entry with type RUN_LVL, BOOT_TIME,
	 OLD_TIME, or NEW_TIME.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read (utmp_data->ut_fd, &utmp_data->ubuf, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      utmp_data->loc_utmp = 0; /* Mark loc_utmp invalid. */
	      __set_errno (ESRCH);
	      return -1;
	    }

	  /* Update position pointer.  */
	  utmp_data->loc_utmp += sizeof (struct utmp);

	  if (id->ut_type == utmp_data->ubuf.ut_type)
	    break;
	}
    }
  else
    {
      /* Search for the next entry with the specified ID and with type
	 INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, or DEAD_PROCESS.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read (utmp_data->ut_fd, &utmp_data->ubuf, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      utmp_data->loc_utmp = 0; /* Mark loc_utmp invalid. */
	      __set_errno (ESRCH);
	      return -1;
	    }

	  /* Update position pointer.  */
	  utmp_data->loc_utmp += sizeof (struct utmp);

	  if ((   utmp_data->ubuf.ut_type == INIT_PROCESS
	       || utmp_data->ubuf.ut_type == LOGIN_PROCESS
	       || utmp_data->ubuf.ut_type == USER_PROCESS
	       || utmp_data->ubuf.ut_type == DEAD_PROCESS)
	      && (strncmp (utmp_data->ubuf.ut_id, id->ut_id, sizeof id->ut_id)
		  == 0))
	    break;
	}
    }

  *utmp = &utmp_data->ubuf;

  return 0;
#else	/* !_HAVE_UT_ID && !_HAVE_UT_TYPE */
  __set_errno (ENOSYS);
  return -1;
#endif
}
weak_alias (__getutid_r, getutid_r)
