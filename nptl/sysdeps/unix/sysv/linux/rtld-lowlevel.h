/* Defintions for lowlevel handling in ld.so.
   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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

#ifndef _RTLD_LOWLEVEL_H
#define  _RTLD_LOWLEVEL_H 1

#include <atomic.h>
#include <lowlevellock.h>


/* Function to wait for variable become zero.  Used in ld.so for
   reference counters.  */
#define __rtld_waitzero(word) \
  do {									      \
    while (1)								      \
      {									      \
	int val = word;							      \
	if (val == 0)							      \
	  break;							      \
	lll_futex_wait (&(word), val, LLL_PRIVATE);			      \
      }									      \
  } while (0)


#define __rtld_notify(word) \
  lll_futex_wake (&(word), 1, LLL_PRIVATE)

#endif
