/* PA-RISC specific definitions for spinlock initializers.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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

/* Initial value of a spinlock.  PA-RISC only implements atomic load
   and clear so this must be non-zero. */
#define __LT_SPINLOCK_INIT ((__atomic_lock_t) { { 1, 1, 1, 1 } })

/* Initialize global spinlocks without cast, generally macro wrapped */
#define __LT_SPINLOCK_ALT_INIT { { 1, 1, 1, 1 } }

/* Macros for lock initializers, not using the above definition.
   The above definition is not used in the case that static initializers
   use this value. */
#define __LOCK_ALT_INITIALIZER { __LT_SPINLOCK_ALT_INIT, 0 }

/* Used to initialize _pthread_fastlock's in non-static case */
#define __LOCK_INITIALIZER ((struct _pthread_fastlock){ __LT_SPINLOCK_INIT, 0 })

/* Used in pthread_atomic initialization */
#define __ATOMIC_INITIALIZER { 0, __LT_SPINLOCK_ALT_INIT }

/* Tell the rest of the code that the initializer is non-zero without
   explaining it's internal structure */
#define __LT_INITIALIZER_NOT_ZERO

