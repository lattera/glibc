/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <bits/libc-lock.h>
#include <errno.h>
#include <stdlib.h>

/* Placeholder for key creation routine from Hurd cthreads library.  */
int
weak_function
cthread_keycreate (key)
     cthread_key_t *key;
{
  __set_errno (ENOSYS);
 *key = -1;
  return -1;
}

/* Placeholder for key retrieval routine from Hurd cthreads library.  */
int
weak_function
cthread_getspecific (key, pval)
     cthread_key_t key;
     void **pval;
{
  *pval = NULL;
  __set_errno (ENOSYS);
  return -1;
}

/* Placeholder for key setting routine from Hurd cthreads library.  */
int
weak_function
cthread_setspecific (key, val)
     cthread_key_t key;
     void *val;
{
  __set_errno (ENOSYS);
  return -1;
}

/* Call cthread_getspecific which gets a pointer to the return value instead
   of just returning it.  */
void *
__libc_getspecific (key)
     cthread_key_t key;
{
  void *val;
  cthread_getspecific (key, &val);
  return val;
}
