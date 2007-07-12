/* setresuid -- set real user ID, effective user ID, and saved-set user ID
   Copyright (C) 2002, 2006 Free Software Foundation, Inc.
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

/* Set the real user ID, effective user ID, and saved-set user ID,
   of the calling process to RUID, EUID, and SUID, respectively.  */
int
__setresuid (uid_t ruid, uid_t euid, uid_t suid)
{
  __set_errno (ENOSYS);
  return -1;
}
libc_hidden_def (__setresuid)
stub_warning (setresuid)

weak_alias (__setresuid, setresuid)
#include <stub-tag.h>
