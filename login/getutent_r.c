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

#include <unistd.h>
#include <utmp.h>


int
__getutent_r (struct utmp **utmp, struct utmp_data *utmp_data)
{
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

  /* Read the next entry.  */
  if (read (utmp_data->ut_fd, &utmp_data->ubuf, sizeof (struct utmp))
      != sizeof (struct utmp))
    return -1;

  /* Update position pointer.  */
  utmp_data->loc_utmp += sizeof (struct utmp);

  *utmp = &utmp_data->ubuf;

  return 0;
}
weak_alias (__getutent_r, getutent_r)
