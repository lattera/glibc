/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
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
#define _ATOMIC_MACHINE_H       1

#include <stdint.h>
#include <features.h>
#include <arch/spr_def.h>

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

#ifdef _LP64
# define __HAVE_64B_ATOMICS 1
#else
/* tilegx32 does have 64-bit atomics, but assumptions in the semaphore
   code mean that unaligned 64-bit atomics will be used if this symbol
   is true, and unaligned atomics are not supported on tile.  */
# define __HAVE_64B_ATOMICS 0
#endif

#define USE_ATOMIC_COMPILER_BUILTINS 0
#define ATOMIC_EXCHANGE_USES_CAS 0

/* Pick appropriate 8- or 4-byte instruction. */
#define __atomic_update(mem, v, op)                                     \
  ((__typeof (*(mem))) (__typeof (*(mem) - *(mem)))                     \
   ((sizeof (*(mem)) == 8) ?                                            \
    __insn_##op ((void *) (mem), (int64_t) (__typeof((v) - (v))) (v)) : \
    (sizeof (*(mem)) == 4) ?                                            \
    __insn_##op##4 ((void *) (mem), (int32_t) (__typeof ((v) - (v))) (v)) : \
    __atomic_error_bad_argument_size()))

#define atomic_compare_and_exchange_val_acq(mem, n, o)                  \
  ({ __insn_mtspr (SPR_CMPEXCH_VALUE, (int64_t) (__typeof ((o) - (o))) (o)); \
     __atomic_update (mem, n, cmpexch); })
#define atomic_exchange_acq(mem, newvalue) \
  __atomic_update (mem, newvalue, exch)
#define atomic_exchange_and_add(mem, value) \
  __atomic_update (mem, value, fetchadd)
#define atomic_and_val(mem, mask) \
  __atomic_update (mem, mask, fetchand)
#define atomic_or_val(mem, mask) \
  __atomic_update (mem, mask, fetchor)
#define atomic_decrement_if_positive(mem) \
  __atomic_update (mem, -1, fetchaddgez)

/* Barrier macro. */
#define atomic_full_barrier() __sync_synchronize()

/* APIs with "release" semantics. */
#define atomic_compare_and_exchange_val_rel(mem, n, o)          \
  ({                                                            \
    atomic_full_barrier ();                                     \
    atomic_compare_and_exchange_val_acq ((mem), (n), (o));      \
  })
#define atomic_exchange_rel(mem, n)                             \
  ({                                                            \
    atomic_full_barrier ();                                     \
    atomic_exchange_acq ((mem), (n));                           \
  })

/* Various macros that should just be synonyms. */
#define catomic_exchange_and_add atomic_exchange_and_add
#define atomic_and(mem, mask) ((void) atomic_and_val ((mem), (mask)))
#define catomic_and atomic_and
#define atomic_or(mem, mask) ((void) atomic_or_val ((mem), (mask)))
#define catomic_or atomic_or

/* atomic_bit_test_set in terms of atomic_or_val. */
#define atomic_bit_test_set(mem, bit)                                   \
  ({ __typeof (*(mem)) __att0_mask = ((__typeof (*(mem))) 1 << (bit));  \
    atomic_or_val ((mem), __att0_mask) & __att0_mask; })

/*
 * This non-existent symbol is called for unsupporrted sizes,
 * indicating a bug in the caller.
 */
extern int __atomic_error_bad_argument_size(void)
  __attribute__ ((warning ("bad sizeof atomic argument")));

#endif /* _ATOMIC_MACHINE_H  */
