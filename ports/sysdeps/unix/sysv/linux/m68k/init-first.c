/* Initialization code run first thing by the ELF startup code.  Linux/m68k.
   Copyright (C) 2010-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Note: linking in vDSO to a static binary requires changes to
   the main GLIBC proper.  Not yet implemented.  */
#ifdef SHARED

#include <dl-vdso.h>
#include <bits/m68k-vdso.h>

static inline void
_libc_vdso_platform_setup (void)
{
  void *p;

  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);

  /* It may happen that rtld didn't initialize the vDSO, so fallback
     to the syscall implementations if _dl_vdso_vsym returns NULL.
     This may happen when a static executable dlopen's a dynamic library.
     This really is nothing more than a workaround for rtld/csu
     deficiency.  Ideally, init code would setup the vDSO for static
     binaries too.  */

  p = _dl_vdso_vsym ("__kernel_read_tp", &linux26);
  if (p != NULL)
    {
      __vdso_read_tp = p;
      __rtld___vdso_read_tp = p;
    }
  else
    assert (__vdso_read_tp == (void *) __vdso_read_tp_stub);

  p = _dl_vdso_vsym ("__kernel_atomic_cmpxchg_32", &linux26);
  if (p != NULL)
    {
      __vdso_atomic_cmpxchg_32 = p;
      __rtld___vdso_atomic_cmpxchg_32 = p;
    }
  else
    assert (__vdso_atomic_cmpxchg_32
	    == (void *) __vdso_atomic_cmpxchg_32_stub);

  p = _dl_vdso_vsym ("__kernel_atomic_barrier", &linux26);
  if (p != NULL)
    {
      __vdso_atomic_barrier = p;
      __rtld___vdso_atomic_barrier = p;
    }
  else
    assert (__vdso_atomic_barrier == (void *) __vdso_atomic_barrier_stub);
}

#define VDSO_SETUP _libc_vdso_platform_setup

#endif /* SHARED */

#include <csu/init-first.c>
