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

#include <alloca.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/file.h>


int
pututline_r (const struct utmp *utmp_ptr, struct utmp_data *utmp_data)
{
  /* Open utmp file if not already done.  */
  if (utmp_data->ut_fd == -1)
    {
      setutent_r (utmp_data);
      if (utmp_data->ut_fd == -1)
	return -1;
    }

  /* Seek position to write.  */
  if (utmp_data->ubuf.ut_type != utmp_ptr->ut_type)
    {
      /* We must not overwrite the data in UTMP_DATA.  */
      struct utmp_data *data_tmp = alloca (sizeof (utmp_data));
      struct utmp *dummy;

      memcpy (data_tmp, utmp_data, sizeof (utmp_data));
      utmp_data = data_tmp;
      
      if (getutid_r (utmp_ptr, &dummy, utmp_data) < 0)
	{
	  if (errno != ESRCH)
	    return -1;

	  utmp_data->loc_utmp = lseek (utmp_data->ut_fd, 0, SEEK_END);
	  if (utmp_data->loc_utmp == -1)
	    return -1;
	}
    }

  /* Position file correctly.  */
  if (utmp_data->loc_utmp > 0
      && lseek (utmp_data->ut_fd, utmp_data->loc_utmp - sizeof (struct utmp),
		SEEK_SET) < 0)
    return -1;

  /* XXX An alternative solution would be to call an SUID root program
     which write the new value.  */
  
  /* Try to lock the file.  */
  if (flock (utmp_data->ut_fd, LOCK_EX | LOCK_NB) < 0 && errno != ENOSYS)
    {
      /* Oh, oh.  The file is already locked.  Wait a bit and try again.  */
      sleep (1);

      /* This time we ignore the error.  */
      (void) flock (utmp_data->ut_fd, LOCK_EX | LOCK_NB);
    }
  
  /* Write the new data.  */
  if (write (utmp_data->ut_fd, &utmp_data->ubuf, sizeof (struct utmp))
      != sizeof (struct utmp))
    return -1;

  /* And unlock the file.  */
  (void) flock (utmp_data->ut_fd, LOCK_UN);

  return 0;
}
