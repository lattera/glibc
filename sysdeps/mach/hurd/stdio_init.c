/* Copyright (C) 1995, 1997, 1998 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <hurd/fd.h>
#include <hurd/io.h>
#include <hurd/term.h>

/* Initialize STREAM as necessary.
   This may change I/O functions, give a buffer, etc.
   If no buffer is allocated, but the bufsize is set,
   the bufsize will be used to allocate the buffer.  */
void
__stdio_init_stream (FILE *stream)
{
  struct hurd_fd *const d = stream->__cookie;
  struct stat statb;
  error_t err;

  if (stream->__buffer != NULL || stream->__userbuf)
    /* If's unbuffered by request, we can't do anything useful.  */
    return;

  /* Find out what sort of file this is.  */
  if (err = HURD_FD_PORT_USE (d, __io_stat (port, &statb)))
    return;

  if (S_ISCHR (statb.st_mode))
    {
      /* It's a character device.
	 Make it line-buffered if it's a terminal.  */
      mach_port_t cttyid;
      err = HURD_FD_PORT_USE (d, __term_getctty (port, &cttyid));
      if (! err)
	{
	  __mach_port_deallocate (__mach_task_self (), cttyid);
	  stream->__linebuf = 1;
	}
    }

  /* Use the block-size field to determine
     the system's optimal buffering size.  */
  stream->__bufsize = statb.st_blksize;
}
