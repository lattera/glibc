/* Copyright (C) 1991, 92, 93, 94, 96 Free Software Foundation, Inc.
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
#include "hurdhost.h"

/* Set the name of the current host to NAME, which is LEN bytes long.
   This call is restricted to the super-user.  */
int
DEFUN(sethostname, (name, len),
      CONST char *name AND size_t len)
{
  /* The host name is just the contents of the file /etc/hostname.  */
  ssize_t n = _hurd_set_host_config ("/etc/hostname", name, len);
  return n < 0 ? -1 : 0;
}
