/* Copyright (C) 2000-2015 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <not-cancel.h>

static int itc_usable;

static int
has_cpuclock (void)
{
  if (__builtin_expect (itc_usable == 0, 0))
    {
      int newval = 1;
      int fd = open_not_cancel_2 ("/proc/sal/itc_drift", O_RDONLY);
      if (__builtin_expect (fd != -1, 1))
	{
	  char buf[16];
	  /* We expect the file to contain a single digit followed by
	     a newline.  If the format changes we better not rely on
	     the file content.  */
	  if (read_not_cancel (fd, buf, sizeof buf) != 2
	      || buf[0] != '0' || buf[1] != '\n')
	    newval = -1;

	  close_not_cancel_no_status (fd);
	}

      itc_usable = newval;
    }

  return itc_usable;
}
