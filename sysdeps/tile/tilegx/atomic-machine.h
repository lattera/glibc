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

#include <arch/spr_def.h>

#ifdef _LP64
# define __HAVE_64B_ATOMICS 1
#else
/* tilegx32 does have 64-bit atomics, but assumptions in the semaphore
   code mean that unaligned 64-bit atomics will be used if this symbol
   is true, and unaligned atomics are not supported on tile.  */
# define __HAVE_64B_ATOMICS 0
#endif

#define USE_ATOMIC_COMPILER_BUILTINS 0

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

#include <sysdeps/tile/atomic-machine.h>

#endif /* atomic-machine.h */
