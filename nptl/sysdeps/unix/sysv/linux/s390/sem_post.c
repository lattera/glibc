/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevelsem.h>
#include <internaltypes.h>

#include <shlib-compat.h>


int
__new_sem_post (sem_t *sem)
{
  int oldval;
  int newval;

  lll_compare_and_swap ((int *) sem, oldval, newval, "lr %2,%1; ahi %2,1");
  lll_futex_wake ((int *) sem, newval);
}
versioned_symbol (libpthread, __new_sem_post, sem_post, GLIBC_2_1);
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
strong_alias (__new_sem_post, __old_sem_post)
compat_symbol (libpthread, __old_sem_post, sem_post, GLIBC_2_0);
#endif
