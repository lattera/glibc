/* Thread package specific definitions of stream lock type.  Generic version.
   Copyright (C) 2000, 2001, 2002 Free Software Foundation, Inc.
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
#include <lowlevellock.h>


typedef struct { int lock; int cnt; void *owner; } _IO_lock_t;

#define _IO_lock_initializer { LLL_MUTEX_LOCK_INITIALIZER, 0, NULL }

#define _IO_lock_init(_name) \
  ((_name) = (_IO_lock_t) _IO_lock_initializer , 0)

#define _IO_lock_fini(_name) \
  ((void) 0)

#define _IO_lock_lock(_name) \
  do {									      \
    void *__self = THREAD_SELF;						      \
    if ((_name).owner != __self)					      \
      {									      \
        lll_mutex_lock ((_name).lock);					      \
        (_name).owner = __self;						      \
      }									      \
    ++(_name).cnt;							      \
  } while (0)

#define _IO_lock_trylock(_name) \
  ({									      \
    int __result = 0;							      \
    void *__self = THREAD_SELF;						      \
    if ((_name).owner != __self)					      \
      {									      \
        if (lll_mutex_trylock ((_name).lock) == 0)			      \
          {								      \
            (_name).owner = __self;					      \
            (_name).cnt = 1;						      \
          }								      \
        else								      \
          __result = EBUSY;						      \
      }									      \
    else								      \
      ++(_name).cnt;							      \
    __result;								      \
  })

#define _IO_lock_unlock(_name) \
  do {									      \
    if (--(_name).cnt == 0)						      \
      {									      \
        (_name).owner = NULL;						      \
        lll_mutex_unlock ((_name).lock);				      \
      }									      \
  } while (0)



#define _IO_cleanup_region_start(_fct, _fp) \
  __libc_cleanup_region_start (((_fp)->_flags & _IO_USER_LOCK) == 0, _fct, _fp)
#define _IO_cleanup_region_start_noarg(_fct) \
  __libc_cleanup_region_start (1, _fct, NULL)
#define _IO_cleanup_region_end(_doit) \
  __libc_cleanup_region_end (_doit)


#endif /* bits/stdio-lock.h */
