/* Atomic operations.  Pure ARM version.
   Copyright (C) 2002-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

void __arm_link_error (void);

/* Use the atomic builtins provided by GCC in case the backend provides
   a pattern to do this efficiently.  */

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
# define atomic_full_barrier() __sync_synchronize ()
#else
# define atomic_full_barrier() __arm_assisted_full_barrier ()
#endif

/* An OS-specific bits/atomic.h file will define this macro if
   the OS can provide something.  If not, we'll fail to build
   with a compiler that doesn't supply the operation.  */
#ifndef __arm_assisted_full_barrier
# define __arm_assisted_full_barrier()  __arm_link_error()
#endif

/* Atomic compare and exchange.  */

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
# define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap ((mem), (oldval), (newval))
#else
# define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  __arm_assisted_compare_and_exchange_val_32_acq ((mem), (newval), (oldval))
#endif

/* We don't support atomic operations on any non-word types.
   So make them link errors.  */
#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

/* An OS-specific bits/atomic.h file will define this macro if
   the OS can provide something.  If not, we'll fail to build
   with a compiler that doesn't supply the operation.  */
#ifndef __arm_assisted_compare_and_exchange_val_32_acq
# define __arm_assisted_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })
#endif
