/* Reentrant function to return the current login name.  Stub version.
   Copyright (C) 1996, 2002 Free Software Foundation, Inc.
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
#include <unistd.h>

/* Return at most NAME_LEN characters of the login name of the user in NAME.
   If it cannot be determined or some other error occurred, return the error
   code.  Otherwise return 0.  */
int
getlogin_r (name, name_len)
     char *name;
     size_t name_len;
{
  __set_errno (ENOSYS);
  return errno;
}
libc_hidden_def (getlogin_r)

stub_warning (getlogin_r)
#include <stub-tag.h>
