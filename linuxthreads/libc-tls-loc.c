/* Special definitions for libc's own exposed thread-specific variables.
   Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <tls.h>

#if USE___THREAD
# include <errno.h>
# include <netdb.h>
# include <resolv.h>

/* These functions have identical definitions in libc.  But the versioned
   dependencies in executables bind them to libpthread.so definitions,
   so we must have some here.  */

int *
__errno_location (void)
{
  return &errno;
}

int *
__h_errno_location (void)
{
  return &h_errno;
}

struct __res_state *
__res_state (void)
{
  return __resp;
}

#endif
