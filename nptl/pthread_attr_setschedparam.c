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

#include <assert.h>
#include <errno.h>
#include <string.h>
#include "pthreadP.h"


int
pthread_attr_setschedparam (attr, param)
     pthread_attr_t *attr;
     const struct sched_param *param;
{
  struct pthread_attr *iattr;
  int max_prio;
  int min_prio;

  assert (sizeof (*attr) >= sizeof (struct pthread_attr));
  iattr = (struct pthread_attr *) attr;

  max_prio = __sched_get_priority_max (iattr->schedpolicy);
  min_prio = __sched_get_priority_min (iattr->schedpolicy);

  /* Catch invalid values.  */
  if (param->sched_priority < min_prio || param->sched_priority > max_prio)
    return EINVAL;

  /* Copy the new values.  */
  memcpy (&iattr->schedparam, param, sizeof (struct sched_param));

  return 0;
}
