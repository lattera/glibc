/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Handling of thread attributes */

#include <unistd.h>
#include "pthread.h"
#include "internals.h"

int pthread_attr_init(pthread_attr_t *attr)
{
  attr->detachstate = PTHREAD_CREATE_JOINABLE;
  attr->schedpolicy = SCHED_OTHER;
  attr->schedparam.sched_priority = 0;
  attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr->scope = PTHREAD_SCOPE_SYSTEM;
  return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
  return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
  if (detachstate < PTHREAD_CREATE_JOINABLE ||
      detachstate > PTHREAD_CREATE_DETACHED)
    return EINVAL;
  attr->detachstate = detachstate;
  return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
  *detachstate = attr->detachstate;
  return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr,
                               const struct sched_param *param)
{
  int max_prio = __sched_get_priority_max(attr->schedpolicy);
  int min_prio = __sched_get_priority_min(attr->schedpolicy);

  if (param->sched_priority < min_prio || param->sched_priority > max_prio)
    return EINVAL;
  attr->schedparam = *param;
  return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr,
                               struct sched_param *param)
{
  *param = attr->schedparam;
  return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
  if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR)
    return EINVAL;
  if (policy != SCHED_OTHER && geteuid() != 0)
    return ENOTSUP;
  attr->schedpolicy = policy;
  return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
  *policy = attr->schedpolicy;
  return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
  if (inherit != PTHREAD_INHERIT_SCHED && inherit != PTHREAD_EXPLICIT_SCHED)
    return EINVAL;
  attr->inheritsched = inherit;
  return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit)
{
  *inherit = attr->inheritsched;
  return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
  switch (scope) {
  case PTHREAD_SCOPE_SYSTEM:
    attr->scope = scope;
    return 0;
  case PTHREAD_SCOPE_PROCESS:
    return ENOTSUP;
  default:
    return EINVAL;
  }
}

int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
  *scope = attr->scope;
  return 0;
}
