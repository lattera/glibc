/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <hurd.h>


/* Open a directory stream on NAME.  */
DIR *
DEFUN(opendir, (name), CONST char *name)
{
  DIR *dirp;
  file_t port;

  port = __file_name_lookup (name, O_RDONLY, 0);
  if (port == MACH_PORT_NULL)
    return NULL;

  /* XXX this port should be deallocated on exec */

  dirp = (DIR *) malloc (sizeof (DIR));
  if (dirp == NULL)
    {
      __mach_port_deallocate (__mach_task_self (), port);
      return NULL;
    }    

  dirp->__port = port;
  dirp->__data = dirp->__ptr = NULL;
  dirp->__entry_data = dirp->__entry_ptr = 0;
  dirp->__allocation = 0;
  dirp->__size = 0;

  return dirp;
}
