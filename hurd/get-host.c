/* Get a host configuration item kept as the whole contents of a file.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fcntl.h>
#include <hurd.h>
#include "hurdhost.h"

ssize_t
_hurd_get_host_config (const char *item, char *buf, size_t buflen)
{
  error_t err;
  char *data;
  mach_msg_type_number_t nread, more;
  file_t config = __file_name_lookup (item, O_RDONLY, 0);
  if (config == MACH_PORT_NULL)
    return -1;

  data = buf;
  err = __io_read (config, &data, &nread, -1, buflen);
  if (! err)
    /* Check if there is more in the file we didn't read.  */
    err = __io_readable (config, &more);
  __mach_port_deallocate (__mach_task_self (), config);
  if (err)
    return __hurd_fail (err);
  if (data != buf)
    {
      memcpy (buf, data, nread);
      __vm_deallocate (__mach_task_self (), (vm_address_t) data, nread);
    }

  /* Remove newlines in case someone wrote the file by hand.  */
  while (buf[nread - 1] == '\n')
    buf[--nread] = '\0';

  if (more)
    /* If we didn't read the whole file, tell the caller to use a bigger
       buffer next time.  */
    return __hurd_fail (ENAMETOOLONG);

  return nread;
}
