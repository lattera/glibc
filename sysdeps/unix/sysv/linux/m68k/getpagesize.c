/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@suse.de>.

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

#include <unistd.h>
#include <sys/param.h>
#include <errno.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Return the system page size.  */
int
__getpagesize ()
{
  extern size_t _dl_pagesize;
#ifdef __NR_getpagesize
  int result;
#endif

  if (_dl_pagesize != 0)
    return _dl_pagesize;

  result = INLINE_SYSCALL (getpagesize, 0);
  /* The only possible error is ENOSYS.  */
  if (result != -1)
    return result;
#endif

  return 4096;
}

weak_alias (__getpagesize, getpagesize)
