/* vfork ABI-compatibility entry points for libpthread.
   Copyright (C) 2014 Free Software Foundation, Inc.
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

#if (SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20) \
     || SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20))

extern __typeof (vfork) __libc_vfork;   /* Defined in libc.  */

# ifdef HAVE_IFUNC

attribute_hidden __attribute__ ((used))
__typeof (vfork) *
vfork_ifunc (void)
{
  return &__libc_vfork;
}

#  ifdef HAVE_ASM_SET_DIRECTIVE
#   define DEFINE_VFORK(name) \
  asm (".set " #name ", vfork_ifunc\n" \
       ".globl " #name "\n" \
       ".type " #name ", %gnu_indirect_function");
#  else
#   define DEFINE_VFORK(name) \
  asm (#name " = vfork_ifunc\n" \
       ".globl " #name "\n" \
       ".type " #name ", %gnu_indirect_function");
#  endif

# else

attribute_hidden
pid_t
vfork_compat (void)
{
  return __libc_vfork ();
}

# define DEFINE_VFORK(name)     weak_alias (vfork_compat, name)

# endif
#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20)
DEFINE_VFORK (vfork)
#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20)
DEFINE_VFORK (__vfork)
#endif
