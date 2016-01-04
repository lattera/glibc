/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <unistd.h>
#include <list.h>
#include <fork.h>
#include <dl-sysdep.h>
#include <tls.h>
#include <string.h>
#include <pthreadP.h>
#include <libc-lock.h>
#include <sysdep.h>
#include <ldsodefs.h>


unsigned long int *__fork_generation_pointer;


#ifdef TLS_MULTIPLE_THREADS_IN_TCB
void
#else
extern int __libc_multiple_threads attribute_hidden;

int *
#endif
internal_function
__libc_pthread_init (unsigned long int *ptr, void (*reclaim) (void),
		     const struct pthread_functions *functions)
{
  /* Remember the pointer to the generation counter in libpthread.  */
  __fork_generation_pointer = ptr;

  /* Called by a child after fork.  */
  __register_atfork (NULL, NULL, reclaim, NULL);

#ifdef SHARED
  /* Copy the function pointers into an array in libc.  This enables
     access with just one memory reference but moreso, it prevents
     hijacking the function pointers with just one pointer change.  We
     "encrypt" the function pointers since we cannot write-protect the
     array easily enough.  */
  union ptrhack
  {
    struct pthread_functions pf;
# define NPTRS (sizeof (struct pthread_functions) / sizeof (void *))
    void *parr[NPTRS];
  } __attribute__ ((may_alias)) const *src;
  union ptrhack *dest;

  src = (const void *) functions;
  dest = (void *) &__libc_pthread_functions;

  for (size_t cnt = 0; cnt < NPTRS; ++cnt)
    {
      void *p = src->parr[cnt];
      PTR_MANGLE (p);
      dest->parr[cnt] = p;
    }
  __libc_pthread_functions_init = 1;
#endif

#ifndef TLS_MULTIPLE_THREADS_IN_TCB
  return &__libc_multiple_threads;
#endif
}

#ifdef SHARED
libc_freeres_fn (freeres_libptread)
{
  if (__libc_pthread_functions_init)
    PTHFCT_CALL (ptr_freeres, ());
}
#endif
