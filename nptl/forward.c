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

#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>

#include <shlib-compat.h>


#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_3_2)
static void *libpthread_handle;


static void
test_loaded (void)
{
  void *h = __libc_dlopen_mode ("libpthread.so.0", RTLD_LAZY | RTLD_NOLOAD);

  libpthread_handle = h ?: (void *) -1l;
}


#define FORWARD3(name, rettype, decl, params, defaction, version) \
rettype									      \
__noexport_##name decl							      \
{									      \
  if (libpthread_handle == NULL)					      \
    test_loaded ();							      \
									      \
  if (libpthread_handle == (void *) -1l)				      \
    defaction;								      \
									      \
  static __typeof (name) *p;						      \
  p = __libc_dlsym (libpthread_handle, #name);				      \
									      \
  return p params;							      \
}									      \
compat_symbol (libc, __noexport_##name, name, version)

#define FORWARD2(name, decl, params, defretval, version) \
  FORWARD3 (name, int, decl, params, return defretval, version)

#define FORWARD(name, decl, params, defretval) \
  FORWARD2 (name, decl, params, defretval, GLIBC_2_0)


FORWARD (pthread_attr_destroy, (pthread_attr_t *attr), (attr), 0);

FORWARD2 (pthread_attr_init, (pthread_attr_t *attr), (attr), 0, GLIBC_2_1);

FORWARD (pthread_attr_getdetachstate,
	 (const pthread_attr_t *attr, int *detachstate), (attr, detachstate),
	 0);
FORWARD (pthread_attr_setdetachstate, (pthread_attr_t *attr, int detachstate),
	 (attr, detachstate), 0);

FORWARD (pthread_attr_getinheritsched,
	 (const pthread_attr_t *attr, int *inherit), (attr, inherit), 0);
FORWARD (pthread_attr_setinheritsched, (pthread_attr_t *attr, int inherit),
	 (attr, inherit), 0);

FORWARD (pthread_attr_getschedparam,
	 (const pthread_attr_t *attr, struct sched_param *param),
	 (attr, param), 0);
FORWARD (pthread_attr_setschedparam,
	 (pthread_attr_t *attr, const struct sched_param *param),
	 (attr, param), 0);

FORWARD (pthread_attr_getschedpolicy,
	 (const pthread_attr_t *attr, int *policy), (attr, policy), 0);
FORWARD (pthread_attr_setschedpolicy, (pthread_attr_t *attr, int policy),
	 (attr, policy), 0);

FORWARD (pthread_attr_getscope,
	 (const pthread_attr_t *attr, int *scope), (attr, scope), 0);
FORWARD (pthread_attr_setscope, (pthread_attr_t *attr, int scope),
	 (attr, scope), 0);


FORWARD (pthread_condattr_destroy, (pthread_condattr_t *attr), (attr), 0);
FORWARD (pthread_condattr_init, (pthread_condattr_t *attr), (attr), 0);


FORWARD (pthread_cond_broadcast, (pthread_cond_t *cond), (cond), 0);

FORWARD (pthread_cond_destroy, (pthread_cond_t *cond), (cond), 0);

FORWARD (pthread_cond_init,
	 (pthread_cond_t *cond, const pthread_condattr_t *cond_attr),
	 (cond, cond_attr), 0);

FORWARD (pthread_cond_signal, (pthread_cond_t *cond), (cond), 0);

FORWARD (pthread_cond_wait, (pthread_cond_t *cond, pthread_mutex_t *mutex),
	 (cond, mutex), 0);


FORWARD (pthread_equal, (pthread_t thread1, pthread_t thread2),
	 (thread1, thread2), 1);


FORWARD3 (pthread_exit, void, (void *retval), (retval), exit (EXIT_SUCCESS),
	  GLIBC_2_0);


FORWARD (pthread_getschedparam,
	 (pthread_t target_thread, int *policy, struct sched_param *param),
	 (target_thread, policy, param), 0);
FORWARD (pthread_setschedparam,
	 (pthread_t target_thread, int policy,
	  const struct sched_param *param), (target_thread, policy, param), 0);


FORWARD (pthread_mutex_destroy, (pthread_mutex_t *mutex), (mutex), 0);

FORWARD (pthread_mutex_init,
	 (pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr),
	 (mutex, mutexattr), 0);

FORWARD (pthread_mutex_lock, (pthread_mutex_t *mutex), (mutex), 0);

FORWARD (pthread_mutex_unlock, (pthread_mutex_t *mutex), (mutex), 0);


FORWARD (pthread_self, (void), (), 0);


FORWARD (pthread_setcancelstate, (int state, int *oldstate), (state, oldstate),
	 0);

FORWARD (pthread_setcanceltype, (int type, int *oldtype), (type, oldtype), 0);


#endif
