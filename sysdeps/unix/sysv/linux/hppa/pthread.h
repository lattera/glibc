/* Copyright (C) 2002-2015 Free Software Foundation, Inc.
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

#include_next <pthread.h>

#ifndef _PTHREAD_H_HPPA_
#define _PTHREAD_H_HPPA_ 1

/* The pthread_cond_t initializer is compatible only with NPTL. We do not
   want to be forwards compatible, we eventually want to drop the code
   that has to clear the old LT initializer.  */
#undef PTHREAD_COND_INITIALIZER
#define PTHREAD_COND_INITIALIZER { { 0, 0, 0, (void *) 0, 0, 0, 0, 0, 0 } }

/* The pthread_mutex_t and pthread_rwlock_t initializers are compatible
   only with NPTL. NPTL assumes pthread_rwlock_t is all zero.  */
#undef PTHREAD_MUTEX_INITIALIZER
#undef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#undef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
#undef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
/* Mutex initializers.  */
#define PTHREAD_MUTEX_INITIALIZER \
  { { 0, 0, 0, 0, { 0, 0, 0, 0 }, 0, { 0 }, 0, 0 } }
#ifdef __USE_GNU
# define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
  { { 0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, { 0, 0, 0, 0 }, 0, { 0 }, 0, 0 } }
# define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP \
  { { 0, 0, 0, PTHREAD_MUTEX_ERRORCHECK_NP, { 0, 0, 0, 0 }, 0, { 0 }, 0, 0 } }
# define PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP \
  { { 0, 0, 0, PTHREAD_MUTEX_ADAPTIVE_NP, { 0, 0, 0, 0 }, 0, { 0 }, 0, 0 } }
#endif

#undef PTHREAD_RWLOCK_INITIALIZER
#undef PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP
/* Read-write lock initializers.  */
#define PTHREAD_RWLOCK_INITIALIZER \
  { { { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
#ifdef __USE_GNU
# define PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP \
  { { { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,\
      0, 0, 0 } }
#endif  /* Unix98 or XOpen2K */

#endif
