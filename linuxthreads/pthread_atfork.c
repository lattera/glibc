/* Copyright (C) 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "internals.h"
#include <fork.h>

/* This is defined by newer gcc version unique for each module.  */
extern void *__dso_handle __attribute__ ((__weak__));


/* Hide the symbol so that no definition but the one locally in the
   executable or DSO is used.  */
int
#ifndef __pthread_atfork
/* Don't mark the compatibility function as hidden.  */
attribute_hidden
#endif
__pthread_atfork (prepare, parent, child)
     void (*prepare) (void);
     void (*parent) (void);
     void (*child) (void);
{
  return __register_atfork (prepare, parent, child,
			    &__dso_handle == NULL ? NULL : __dso_handle);
}
#ifndef __pthread_atfork
extern int pthread_atfork (void (*prepare) (void), void (*parent) (void),
			   void (*child) (void)) attribute_hidden;
strong_alias (__pthread_atfork, pthread_atfork)
#endif
