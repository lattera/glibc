/* Get system load averages.  Linux (/proc/loadavg) version.
   Copyright (C) 1999, 2000, 2001, 2003 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <not-cancel.h>

/* Put the 1 minute, 5 minute and 15 minute load averages
   into the first NELEM elements of LOADAVG.
   Return the number written (never more than 3, but may be less than NELEM),
   or -1 if an error occurred.  */

int
getloadavg (double loadavg[], int nelem)
{
  int fd;

  fd = open_not_cancel_2 ("/proc/loadavg", O_RDONLY);
  if (fd < 0)
    return -1;
  else
    {
      char buf[65], *p;
      ssize_t nread;
      int i;

      nread = read_not_cancel (fd, buf, sizeof buf - 1);
      close_not_cancel_no_status (fd);
      if (nread <= 0)
	return -1;
      buf[nread - 1] = '\0';

      if (nelem > 3)
	nelem = 3;
      p = buf;
      for (i = 0; i < nelem; ++i)
	{
	  char *endp;
	  loadavg[i] = __strtod_l (p, &endp, &_nl_C_locobj);
	  if (endp == p)
	    /* This should not happen.  The format of /proc/loadavg
	       must have changed.  Don't return with what we have,
	       signal an error.  */
	    return -1;
	  p = endp;
	}

      return i;
    }
}
