/* Copyright (C) 1992, 93, 94, 95, 97, 98 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <errno.h>
#include <sys/types.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <string.h>
#include <unistd.h>

ssize_t
__getdirentries (int fd,
		 char *buf,
		 size_t nbytes,
		 off_t *basep)
{
  error_t err;
  int entriesread;
  char *data = buf;
  mach_msg_type_number_t bytesread = nbytes;

  /* Fault before taking any locks.  */
  *(volatile off_t *) basep = *basep;

  err = HURD_DPORT_USE (fd, __dir_readdir (port, &data, &bytesread,
					   *basep, -1, nbytes, &entriesread));
  if (err)
    return __hurd_dfail (fd, err);

  if (data != buf)
    {
      size_t copy = bytesread;
      if (copy > nbytes)
	/* The server has a violated the dir_readdir protocol.  */
	copy = nbytes;
      memcpy (buf, data, copy);
      __vm_deallocate (__mach_task_self (), (vm_address_t) data, bytesread);
      bytesread = copy;
    }

  *basep += entriesread;

  return bytesread;
}

weak_alias (__getdirentries, getdirentries)
