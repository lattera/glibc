/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

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

/* Ugly hack to avoid the declaration of pthread_spin_init.  */
#define pthread_spin_init pthread_spin_init_XXX
#include "pthreadP.h"
#undef pthread_spin_init

int
pthread_spin_unlock (pthread_spinlock_t *lock)
{
  __asm __volatile ("   xc  %O0(4,%R0),%0\n"
		    "   bcr 15,0"
		    : "=Q" (*lock) : "m" (*lock) : "cc" );
  return 0;
}
strong_alias (pthread_spin_unlock, pthread_spin_init)
