/* Lock elision tunable parameters.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <config.h>
#include <pthreadP.h>
#include <elision-conf.h>
#include <unistd.h>
#include <dl-procinfo.h>

/* Reasonable initial tuning values, may be revised in the future.
   This is a conservative initial value.  */

struct elision_config __elision_aconf =
  {
    /* How often to not attempt to use elision if a transaction aborted
       because the lock is already acquired.  Expressed in number of lock
       acquisition attempts.  */
    .skip_lock_busy = 3,
    /* How often to not attempt to use elision if a transaction aborted due
       to reasons other than other threads' memory accesses.  Expressed in
       number of lock acquisition attempts.  */
    .skip_lock_internal_abort = 3,
    /* How often to not attempt to use elision if a lock used up all retries
       without success.  Expressed in number of lock acquisition attempts.  */
    .skip_lock_out_of_tbegin_retries = 3,
    /* How often we try using elision if there is chance for the transaction
       to finish execution (e.g., it wasn't aborted due to the lock being
       already acquired.  */
    .try_tbegin = 3,
    /* Same as SKIP_LOCK_INTERNAL_ABORT but for trylock.  */
    .skip_trylock_internal_abort = 3,
  };

/* Force elision for all new locks.  This is used to decide whether existing
   DEFAULT locks should be automatically upgraded to elision in
   pthread_mutex_lock().  Disabled for suid programs.  Only used when elision
   is available.  */

int __pthread_force_elision attribute_hidden = 0;

/* Initialize elison.  */

static void
elision_init (int argc __attribute__ ((unused)),
	      char **argv  __attribute__ ((unused)),
	      char **environ)
{
  /* Set when the CPU and the kernel supports transactional execution.
     When false elision is never attempted.  */
  int elision_available = (GLRO (dl_hwcap) & HWCAP_S390_TE) ? 1 : 0;

  __pthread_force_elision = __libc_enable_secure ? 0 : elision_available;
}

#ifdef SHARED
# define INIT_SECTION ".init_array"
# define MAYBE_CONST
#else
# define INIT_SECTION ".preinit_array"
# define MAYBE_CONST const
#endif

void (*MAYBE_CONST __pthread_init_array []) (int, char **, char **)
  __attribute__ ((section (INIT_SECTION), aligned (sizeof (void *)))) =
{
  &elision_init
};
