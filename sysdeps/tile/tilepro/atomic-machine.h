/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _ATOMIC_MACHINE_H
#define _ATOMIC_MACHINE_H	1

#include <asm/unistd.h>

#define __HAVE_64B_ATOMICS 0
#define USE_ATOMIC_COMPILER_BUILTINS 0

/* 32-bit integer compare-and-exchange. */
static __inline __attribute__ ((always_inline))
int __atomic_cmpxchg_32 (volatile int *mem, int newval, int oldval)
{
  int result;
  __asm__ __volatile__ ("swint1"
                        : "=R00" (result), "=m" (*mem)
                        : "R10" (__NR_FAST_cmpxchg), "R00" (mem),
                          "R01" (oldval), "R02" (newval), "m" (*mem)
                        : "r20", "r21", "r22", "r23", "r24",
                          "r25", "r26", "r27", "r28", "r29", "memory");
  return result;
}

#define atomic_compare_and_exchange_val_acq(mem, n, o)                  \
  ({                                                                    \
    if (sizeof (*(mem)) != 4)                                           \
      __atomic_error_bad_argument_size ();                              \
    (__typeof (*(mem)))                                                 \
      __atomic_cmpxchg_32 ((int *) (mem), (int) (n), (int) (o));        \
  })

/* Atomically compute:
     int old = *ptr;
     *ptr = (old & mask) + addend;
     return old;  */

static __inline __attribute__ ((always_inline))
int __atomic_update_32 (volatile int *mem, int mask, int addend)
{
  int result;
  __asm__ __volatile__ ("swint1"
                        : "=R00" (result), "=m" (*mem)
                        : "R10" (__NR_FAST_atomic_update), "R00" (mem),
                          "R01" (mask), "R02" (addend), "m" (*mem)
                        : "r20", "r21", "r22", "r23", "r24",
                          "r25", "r26", "r27", "r28", "r29", "memory");
  return result;
}

/* Size-checked verson of __atomic_update_32. */
#define __atomic_update(mem, mask, addend)                              \
  ({                                                                    \
    if (sizeof (*(mem)) != 4)                                           \
      __atomic_error_bad_argument_size ();                              \
    (__typeof (*(mem)))                                                 \
      __atomic_update_32 ((int *) (mem), (int) (mask), (int) (addend)); \
  })

#define atomic_exchange_acq(mem, newvalue)              \
  __atomic_update ((mem), 0, (newvalue))
#define atomic_exchange_and_add(mem, value)             \
  __atomic_update ((mem), -1, (value))
#define atomic_and_val(mem, mask)                       \
  __atomic_update ((mem), (mask), 0)
#define atomic_or_val(mem, mask)                        \
  ({ __typeof (mask) __att1_v = (mask);                 \
    __atomic_update ((mem), ~__att1_v, __att1_v); })

/*
 * We must use the kernel atomics for atomic_store, since otherwise an
 * unsynchronized store could become visible after another core's
 * kernel-atomic implementation had read the memory word in question,
 * but before it had written the updated value to it, which would
 * cause the unsynchronized store to be lost.
 */
#define atomic_store_relaxed(mem, val) atomic_exchange_acq (mem, val)
#define atomic_store_release(mem, val) atomic_exchange_rel (mem, val)

#include <sysdeps/tile/atomic-machine.h>

#endif /* atomic-machine.h */
