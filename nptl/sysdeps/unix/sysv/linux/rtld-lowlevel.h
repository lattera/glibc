/* Defintions for lowlevel handling in ld.so.
   Copyright (C) 2006 Free Software Foundation, Inc.
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

#ifndef _RTLD_LOWLEVEL_H
#define  _RTLD_LOWLEVEL_H 1

#include <atomic.h>
#include <lowlevellock.h>


/* Special multi-reader lock used in ld.so.  */
#define __RTLD_MRLOCK_WRITER 1
#define __RTLD_MRLOCK_RWAIT 2
#define __RTLD_MRLOCK_WWAIT 4
#define __RTLD_MRLOCK_RBITS \
  ~(__RTLD_MRLOCK_WRITER | __RTLD_MRLOCK_RWAIT | __RTLD_MRLOCK_WWAIT)
#define __RTLD_MRLOCK_INC 8
#define __RTLD_MRLOCK_TRIES 5


typedef int __rtld_mrlock_t;


#define __rtld_mrlock_define(CLASS,NAME) \
  CLASS __rtld_mrlock_t NAME;


#define _RTLD_MRLOCK_INITIALIZER 0
#define __rtld_mrlock_initialize(NAME) \
  (void) ((NAME) = 0


#define __rtld_mrlock_lock(lock) \
  do {									      \
    __label__ out;							      \
    while (1)								      \
      {									      \
	int oldval;							      \
	for (int tries = 0; tries < __RTLD_MRLOCK_TRIES; ++tries)	      \
	  {								      \
	    oldval = lock;						      \
	    while (__builtin_expect ((oldval				      \
				      & (__RTLD_MRLOCK_WRITER		      \
					 | __RTLD_MRLOCK_WWAIT))	      \
				     == 0, 1))				      \
	      {								      \
		int newval = ((oldval & __RTLD_MRLOCK_RBITS)		      \
			      + __RTLD_MRLOCK_INC);			      \
		int ret = catomic_compare_and_exchange_val_acq (&(lock),      \
								newval,	      \
								oldval);      \
		if (__builtin_expect (ret == oldval, 1))		      \
		  goto out;						      \
	      }								      \
	    atomic_delay ();						      \
	  }								      \
	if ((oldval & __RTLD_MRLOCK_RWAIT) == 0)			      \
	  {								      \
	    catomic_or (&(lock), __RTLD_MRLOCK_RWAIT);			      \
	    oldval |= __RTLD_MRLOCK_RWAIT;				      \
	  }								      \
	lll_futex_wait (lock, oldval);					      \
      }									      \
  out:;									      \
  } while (0)


#define __rtld_mrlock_unlock(lock) \
  do {									      \
    int oldval = catomic_exchange_and_add (&(lock), -__RTLD_MRLOCK_INC);      \
    if (__builtin_expect ((oldval					      \
			   & (__RTLD_MRLOCK_RBITS | __RTLD_MRLOCK_WWAIT))     \
			  == (__RTLD_MRLOCK_INC | __RTLD_MRLOCK_WWAIT), 0))   \
      /* We have to wake all threads since there might be some queued	      \
	 readers already.  */						      \
      lll_futex_wake (&(lock), 0x7fffffff);				      \
  } while (0)


/* There can only ever be one thread trying to get the exclusive lock.  */
#define __rtld_mrlock_change(lock) \
  do {									      \
    __label__ out;							      \
    while (1)								      \
      {									      \
	int oldval;							      \
	for (int tries = 0; tries < __RTLD_MRLOCK_TRIES; ++tries)	      \
	  {								      \
	    oldval = lock;						      \
	    while (__builtin_expect ((oldval & __RTLD_MRLOCK_RBITS) == 0, 1))\
	      {								      \
		int newval = ((oldval & __RTLD_MRLOCK_RWAIT)		      \
			      + __RTLD_MRLOCK_WRITER);			      \
		int ret = catomic_compare_and_exchange_val_acq (&(lock),      \
							       newval,	      \
							       oldval);	      \
		if (__builtin_expect (ret == oldval, 1))		      \
		  goto out;						      \
	      }								      \
	    atomic_delay ();						      \
	  }								      \
	catomic_or (&(lock), __RTLD_MRLOCK_WWAIT);			      \
	oldval |= __RTLD_MRLOCK_WWAIT;					      \
	lll_futex_wait (lock, oldval);					      \
      }									      \
  out:;									      \
  } while (0)


#define __rtld_mrlock_done(lock) \
  do {				 \
    int oldval = catomic_exchange_and_add (&(lock), -__RTLD_MRLOCK_WRITER);   \
    if (__builtin_expect ((oldval & __RTLD_MRLOCK_RWAIT) != 0, 0))	      \
      lll_futex_wake (&(lock), 0x7fffffff);				      \
  } while (0)


/* Function to wait for variable become zero.  Used in ld.so for
   reference counters.  */
#define __rtld_waitzero(word) \
  do {									      \
    while (1)								      \
      {									      \
	int val = word;							      \
	if (val == 0)							      \
	  break;							      \
	lll_futex_wait (&(word), val);					      \
      }									      \
  } while (0)


#define __rtld_notify(word) \
  lll_futex_wake (&(word), 1)

#endif
