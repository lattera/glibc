/* Get frequency of the system processor.  powerpc/Linux version.
   Copyright (C) 2000, 2001, 2005 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <libc-internal.h>
#include <sysdep.h>
#include <bits/libc-vdso.h>

hp_timing_t
__get_clockfreq (void)
{
  /* We read the information from the /proc filesystem.  /proc/cpuinfo
     contains at least one line like:
     timebase        : 33333333
     We search for this line and convert the number into an integer.  */
  static hp_timing_t timebase_freq;
  hp_timing_t result = 0L;

  /* If this function was called before, we know the result.  */
  if (timebase_freq != 0)
    return timebase_freq;

  /* If we can use the vDSO to obtain the timebase even better.  */
#ifdef SHARED
  INTERNAL_SYSCALL_DECL (err);
  timebase_freq = INTERNAL_VSYSCALL_NO_SYSCALL_FALLBACK (get_tbfreq, err, 0);
  if (INTERNAL_SYSCALL_ERROR_P (timebase_freq, err)
      && INTERNAL_SYSCALL_ERRNO (timebase_freq, err) == ENOSYS)
#endif
    {
      int fd = open ("/proc/cpuinfo", O_RDONLY);

      if (__builtin_expect (fd != -1, 1))
	{
	  /* The timebase will be in the 1st 1024 bytes for systems with up
	     to 8 processors.  If the first read returns less then 1024
	     bytes read,  we have the whole cpuinfo and can start the scan.
	     Otherwise we will have to read more to insure we have the
	     timebase value in the scan.  */
	  char buf[1024];
	  ssize_t n;

	  n = read (fd, buf, sizeof (buf));
	  if (n == sizeof (buf))
	    {
	      /* We are here because the 1st read returned exactly sizeof
	         (buf) bytes.  This implies that we are not at EOF and may
	         not have read the timebase value yet.  So we need to read
	         more bytes until we know we have EOF.  We copy the lower
	         half of buf to the upper half and read sizeof (buf)/2
	         bytes into the lower half of buf and repeat until we
	         reach EOF.  We can assume that the timebase will be in
	         the last 512 bytes of cpuinfo, so two 512 byte half_bufs
	         will be sufficient to contain the timebase and will
	         handle the case where the timebase spans the half_buf
	         boundry.  */
	      const ssize_t half_buf = sizeof (buf) / 2;
	      while (n >= half_buf)
		{
		  memcpy (buf, buf + half_buf, half_buf);
		  n = read (fd, buf + half_buf, half_buf);
		}
	      if (n >= 0)
		n += half_buf;
	    }

	  if (__builtin_expect (n, 1) > 0)
	    {
	      char *mhz = memmem (buf, n, "timebase", 7);

	      if (__builtin_expect (mhz != NULL, 1))
		{
		  char *endp = buf + n;

		  /* Search for the beginning of the string.  */
		  while (mhz < endp && (*mhz < '0' || *mhz > '9')
			 && *mhz != '\n')
		    ++mhz;

		  while (mhz < endp && *mhz != '\n')
		    {
		      if (*mhz >= '0' && *mhz <= '9')
			{
			  result *= 10;
			  result += *mhz - '0';
			}

		      ++mhz;
		    }
		}
	      timebase_freq = result;
	    }
	  close (fd);
	}
    }

  return timebase_freq;
}
