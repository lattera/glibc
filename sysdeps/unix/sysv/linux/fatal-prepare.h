/* Copyright (C) 2003-2013 Free Software Foundation, Inc.
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

#include <pthread.h>

/* We have to completely disable cancellation.  assert() must not be a
   cancellation point but the implementation uses write() etc.  */
#ifdef SHARED
# include <pthread-functions.h>
# define FATAL_PREPARE \
  {									      \
    if (__libc_pthread_functions_init)					      \
      PTHFCT_CALL (ptr_pthread_setcancelstate, (PTHREAD_CANCEL_DISABLE,	      \
						NULL));			      \
  }
#else
# pragma weak pthread_setcancelstate
# define FATAL_PREPARE \
  {									      \
    if (pthread_setcancelstate != NULL)					      \
      pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);		      \
  }
#endif
