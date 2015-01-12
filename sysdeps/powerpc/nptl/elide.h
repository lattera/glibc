/* elide.h: Generic lock elision support for powerpc.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#ifndef ELIDE_PPC_H
# define ELIDE_PPC_H

#ifdef ENABLE_LOCK_ELISION
# include <htm.h>
# include <elision-conf.h>

/* Returns true if the lock defined by is_lock_free as elided.
   ADAPT_COUNT is a pointer to per-lock state variable. */

static inline bool
__elide_lock (uint8_t *adapt_count, int is_lock_free)
{
  if (*adapt_count > 0)
    {
      (*adapt_count)--;
      return false;
    }

  for (int i = __elision_aconf.try_tbegin; i > 0; i--)
    {
      if (__builtin_tbegin (0))
	{
	  if (is_lock_free)
	    return true;
	  /* Lock was busy.  */
	  __builtin_tabort (_ABORT_LOCK_BUSY);
	}
      else
	{
	  /* A persistent failure indicates that a retry will probably
	     result in another failure.  Use normal locking now and
	     for the next couple of calls.  */
	  if (_TEXASRU_FAILURE_PERSISTENT (__builtin_get_texasru ()))
	    {
	      if (__elision_aconf.skip_lock_internal_abort > 0)
		*adapt_count = __elision_aconf.skip_lock_internal_abort;
	      break;
	    }
	  /* Same logic as above, but for a number of temporary failures in a
	     a row.  */
	  else if (__elision_aconf.skip_lock_out_of_tbegin_retries > 0
		   && __elision_aconf.try_tbegin > 0)
	    *adapt_count = __elision_aconf.skip_lock_out_of_tbegin_retries;
	}
     }

  return false;
}

# define ELIDE_LOCK(adapt_count, is_lock_free) \
  __elide_lock (&(adapt_count), is_lock_free)


static inline bool
__elide_trylock (uint8_t *adapt_count, int is_lock_free, int write)
{
  if (__elision_aconf.try_tbegin > 0)
    {
      if (write)
	__builtin_tabort (_ABORT_NESTED_TRYLOCK);
      return __elide_lock (adapt_count, is_lock_free);
    }
  return false;
}

# define ELIDE_TRYLOCK(adapt_count, is_lock_free, write)	\
  __elide_trylock (&(adapt_count), is_lock_free, write)


static inline bool
__elide_unlock (int is_lock_free)
{
  if (is_lock_free)
    {
      __builtin_tend (0);
      return true;
    }
  return false;
}

# define ELIDE_UNLOCK(is_lock_free) \
  __elide_unlock (is_lock_free)

# else

# define ELIDE_LOCK(adapt_count, is_lock_free) 0
# define ELIDE_TRYLOCK(adapt_count, is_lock_free, write) 0
# define ELIDE_UNLOCK(is_lock_free) 0

#endif /* ENABLE_LOCK_ELISION  */

#endif
