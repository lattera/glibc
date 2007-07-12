/* Tests for AIO in librt.
   Copyright (C) 1998, 2000, 2002, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int
set_o_direct (int fd)
{
  int ret = -1;
#ifdef O_DIRECT
  if (fcntl (fd, F_SETFL, fcntl (fd, F_GETFL) | O_DIRECT) >= 0)
    {
      int pgsz = sysconf (_SC_PAGESIZE);
      char *buf = mmap (NULL, 16 * pgsz, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANON, -1, 0);
      if (buf != MAP_FAILED)
	{
	  memset (buf, 0, 16 * pgsz);
	  for (int sz = 256; sz <= 16 * pgsz; sz *= 2)
	    if (write (fd, buf, sz) > 0)
	      {
		ret = sz;
		break;
	      }
	  ftruncate64 (fd, 0);
	  munmap (buf, 16 * pgsz);
	}
      if (ret < 0)
	fcntl (fd, F_SETFL, fcntl (fd, F_GETFL) & ~O_DIRECT);
    }
#endif
  return ret;
}
