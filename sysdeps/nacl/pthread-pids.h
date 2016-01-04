/* Initialize pid and tid fields of struct pthread.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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


/* NaCl has no concept of PID or TID, nor even any notion of an
   identifier for a thread within the process.  But various places in
   the NPTL implementation rely on using the 'tid' field of the TCB
   (struct pthread) as an identifier that is unique at least among all
   live threads in the process.  So we must synthesize some number to
   use.  Conveniently, the 'pthread_t' value itself is already unique
   in exactly this way (because it's the 'struct pthread' pointer).

   The only wrinkle is that 'tid' is a (32-bit) 'int' and its high
   (sign) bit is used for special purposes, so we must be absolutely
   sure that we never use a pointer value with the high bit set.  (It
   also cannot be zero, but zero is never a valid pointer anyway.)
   The NaCl sandbox models for 32-bit machines limit the address space
   to less than 3GB (in fact, to 1GB), so it's already impossible that
   a valid pointer will have its high bit set.  But the NaCl x86-64
   sandbox model allows a full 4GB of address space, so we cannot
   assume that an arbitrary pointer value will not have the high bit
   set.  Conveniently, there are always unused bits in the pointer
   value for a 'struct pthread', because it is always aligned to at
   least 32 bits and so the low bits are always zero.  Hence, we can
   safely avoid the danger of a nonzero high bit just by shifting the
   pointer value right.  */

static inline int
__nacl_get_tid (struct pthread *pd)
{
  uintptr_t id = (uintptr_t) pd;
  int tid = id >> 1;
  assert ((id & 1) == 0);
  assert (sizeof id == sizeof tid);
  assert (tid > 0);
  /* This ensures that NACL_EXITING_TID (lowlevellock.h) can never
     be a valid TID value.  */
  assert ((tid & 1) == 0);
  return tid;
}


/* Initialize PD->pid and PD->tid for the initial thread.  If there is
   setup required to arrange that __exit_thread causes PD->tid to be
   cleared and futex-woken, then this function should do that as well.  */
static inline void
__pthread_initialize_pids (struct pthread *pd)
{
  pd->tid = __nacl_get_tid (pd);
  pd->pid = -1;
}
