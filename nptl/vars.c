/* Copyright (C) 2004-2016 Free Software Foundation, Inc.
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

#include <pthreadP.h>
#include <stdlib.h>
#include <tls.h>
#include <unistd.h>

/* Default thread attributes for the case when the user does not
   provide any.  */
struct pthread_attr __default_pthread_attr attribute_hidden;

/* Mutex protecting __default_pthread_attr.  */
int __default_pthread_attr_lock = LLL_LOCK_INITIALIZER;

/* Flag whether the machine is SMP or not.  */
int __is_smp attribute_hidden;

#ifndef TLS_MULTIPLE_THREADS_IN_TCB
/* Variable set to a nonzero value either if more than one thread runs or ran,
   or if a single-threaded process is trying to cancel itself.  See
   nptl/descr.h for more context on the single-threaded process case.  */
int __pthread_multiple_threads attribute_hidden;
#endif

/* Table of the key information.  */
struct pthread_key_struct __pthread_keys[PTHREAD_KEYS_MAX]
  __attribute__ ((nocommon));
hidden_data_def (__pthread_keys)
