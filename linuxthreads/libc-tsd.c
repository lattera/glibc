/* Special hack used to build link-time libc.so object for linking libpthread.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
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
#include <resolv.h>

#if ! USE___THREAD

/* Special hack used to build link-time libc.so object for linking libpthread.
   See Makefile comments near libc_pic_lite.os rule for what this is for.  */

# undef _res

int _errno;
int _h_errno;
struct __res_state _res;

#endif

int
__res_maybe_init (res_state resp, int preinit)
{
  return -1;
}
libc_hidden_def (__res_maybe_init)
