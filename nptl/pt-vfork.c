/* vfork ABI-compatibility entry points for libpthread.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <shlib-compat.h>

/* libpthread used to have its own vfork implementation that differed
   from libc's only in having a pointless micro-optimization.  There
   is no longer any use to having a separate copy in libpthread, but
   the historical ABI requires it.  For static linking, there is no
   need to provide anything here--the libc version will be linked in.
   For shared library ABI compatibility, there must be __vfork and
   vfork symbols in libpthread.so; so we define them using IFUNC to
   redirect to the libc function.  */

/* Note! If the architecture doesn't support IFUNC, then we need an
   alternate target-specific mechanism to implement this.  So we just
   assume IFUNC here and require that the target override this file
   if necessary.

   If the architecture can assume all supported versions of gcc will
   produce a tail-call to __libc_vfork, consider including the version
   in sysdeps/unix/sysv/linux/aarch64/pt-vfork.c.  */

#if !HAVE_IFUNC
# error "must write pt-vfork for this machine or get IFUNC support"
#endif

#if (SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20) \
     || SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20))

extern __typeof (vfork) __libc_vfork;   /* Defined in libc.  */

# undef INIT_ARCH
# define INIT_ARCH()
# define DEFINE_VFORK(name) libc_ifunc (name, &__libc_vfork)

#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20)
extern __typeof(vfork) vfork_ifunc;
DEFINE_VFORK (vfork_ifunc)
compat_symbol (libpthread, vfork_ifunc, vfork, GLIBC_2_0);
#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20)
extern __typeof(vfork) __vfork_ifunc;
DEFINE_VFORK (__vfork_ifunc)
compat_symbol (libpthread, __vfork_ifunc, __vfork, GLIBC_2_1_2);
#endif
