/* Thread package specific definitions of stream lock type.  Stub version.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

#ifndef _BITS_STDIO_LOCK_H
#define _BITS_STDIO_LOCK_H 1

#include <bits/libc-lock.h>

__libc_lock_define (typedef, _IO_lock_t)

/* We need recursive (counting) mutexes.  */
#define _IO_lock_initializer ...
#error libio needs recursive mutexes for _IO_MTSAFE_IO


#define _IO_cleanup_region_start(_fct, _fp) \
     __libc_cleanup_region_start (_fct, _fp)
#define _IO_cleanup_region_end(_doit) \
     __libc_cleanup_region_end (_doit)
#define _IO_lock_init(_name) \
     __libc_lock_init_recursive (_name)
#define _IO_lock_fini(_name) \
     __libc_lock_fini_recursive (_name)
#define _IO_lock_lock(_name) \
     __libc_lock_lock (_name)
#define _IO_lock_unlock(_name) \
     __libc_lock_unlock (_name)


#endif /* bits/stdio-lock.h */
