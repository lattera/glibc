/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/paths.h>
#include <fcntl.h>
#include <string.h>

/* Read the contents of the symbolic link FILE_NAME into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
ssize_t
DEFUN(__readlink, (file_name, buf, len),
      CONST char *file_name AND char *buf AND size_t len)
{
  error_t err;
  file_t file;
  char mybuf[2048], *transp = mybuf;
  mach_msg_type_number_t translen = sizeof (mybuf);

  file = __file_name_lookup (file_name, O_NOTRANS, 0);
  if (file == MACH_PORT_NULL)
    return -1;

  err = __file_get_translator (file, &transp, &translen);
  __mach_port_deallocate (__mach_task_self (), file);

  if (err)
    return __hurd_fail (err);

  if (translen < sizeof (_HURD_SYMLINK) ||
      memcmp (transp, _HURD_SYMLINK, sizeof (_HURD_SYMLINK)))
    /* The file is not actually a symlink.  */
    err = EINVAL;
  else
    {
      /* This is a symlink; its translator is "/hurd/symlink\0target\0".  */
      if (len >= translen - sizeof (_HURD_SYMLINK))
	{
	  len = translen - sizeof (_HURD_SYMLINK);
	  if (transp[translen - 1] == '\0')
	    /* Remove the null terminator.  */
	    --len;
	}
      if (buf == NULL)
	/* This call is just to find out how large a buffer is required.  */
	len = translen - sizeof (_HURD_SYMLINK) - 1;
      else
	/* Copy into the user's buffer.  */
	memcpy (buf, transp + sizeof (_HURD_SYMLINK), len);
    }

  if (transp != mybuf)
    __vm_deallocate (__mach_task_self (), (vm_address_t) transp, translen);

  return err ? __hurd_fail (err) : len;
}

weak_alias (__readlink, readlink)
