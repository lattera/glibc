/* Thread package specific definitions of stream lock type.  Hurd version.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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

/* We need recursive (counting) mutexes.  Since cthreads doesn't really
   have them, we implement them just for libio.  The implementation is
   partially here and partially in Hurd's version of cthreads (i.e. the
   libthreads library, libthreads/lockfile.c).  */

#if (_LIBC - 0) || (_CTHREADS_ - 0)
#include <cthreads.h>

struct _IO_cthreads_lock
{
  struct mutex mutex;
  cthread_t owner;
  unsigned int count;
};
#else
struct _IO_cthreads_lock;
#endif
typedef struct _IO_cthreads_lock _IO_lock_t;

#define _IO_lock_initializer	{ MUTEX_INITIALIZER, 0, 0 }

#define _IO_lock_init(_name) ({ (_name) = (_IO_lock_t) _IO_lock_initializer; })
#define _IO_lock_fini(_name) ((void) 0)	/* nothing to do */

/* These are in fact only used for `list_all_lock' (libio/genops.c),
   which does not need a recursive lock.  The per-FILE locks are only
   accessed through _IO_flockfile et al, which Hurd's libthreads overrides.  */
#define _IO_lock_lock(_name) __libc_lock_lock ((_name).mutex)
#define _IO_lock_unlock(_name) __libc_lock_unlock ((_name).mutex)

#ifdef _LIBC
#include <bits/libc-lock.h>

#define _IO_cleanup_region_start(_fct, _fp) \
     __libc_cleanup_region_start (_fct, _fp)
#define _IO_cleanup_region_start_noarg(_fct) \
     __libc_cleanup_region_start (_fct, NULL)
#define _IO_cleanup_region_end(_doit) \
     __libc_cleanup_region_end (_doit)
#endif


#endif /* bits/stdio-lock.h */
