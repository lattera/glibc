/* Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _BITS_ATOMIC_H
#define _BITS_ATOMIC_H	1

#include <arch/spr_def.h>

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

#include <sysdeps/tile/bits/atomic.h>

#endif /* bits/atomic.h */
