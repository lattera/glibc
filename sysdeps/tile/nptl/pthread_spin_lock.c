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

#include "pthreadP.h"
#include <arch/spr_def.h>
#include <atomic.h>

/* Bound point for bounded exponential backoff */
#define BACKOFF_MAX 2048

/* Initial cycle delay for exponential backoff */
#define BACKOFF_START 32

#ifdef __tilegx__
/* Use cmpexch() after the initial fast-path exch to avoid
   invalidating the cache line of the lock holder.  */
# define TNS(p) atomic_exchange_acq((p), 1)
# define CMPTNS(p) atomic_compare_and_exchange_val_acq((p), 1, 0)
#else
# define TNS(p) __insn_tns(p)
# define CMPTNS(p) __insn_tns(p)
# define SPR_CYCLE SPR_CYCLE_LOW   /* The low 32 bits are sufficient. */
#endif

int
pthread_spin_lock (pthread_spinlock_t *lock)
{
  if (__builtin_expect (TNS (lock) != 0, 0))
    {
      unsigned int backoff = BACKOFF_START;
      while (CMPTNS (lock) != 0)
        {
          unsigned int start = __insn_mfspr (SPR_CYCLE);
          while (__insn_mfspr (SPR_CYCLE) - start < backoff)
            ;
          if (backoff < BACKOFF_MAX)
            backoff *= 2;
        }
    }
  return 0;
}
