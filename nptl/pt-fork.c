/* ABI compatibility for 'fork' symbol in libpthread ABI.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <shlib-compat.h>

/* libpthread once had its own fork, though there was no apparent reason
   for it.  There is no use in having a separate symbol in libpthread, but
   the historical ABI requires it.  For static linking, there is no need to
   provide anything here--the libc version will be linked in.  For shared
   library ABI compatibility, there must be __fork and fork symbols in
   libpthread.so; so we define them using IFUNC to redirect to the libc
   function.  */

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_22)

# if HAVE_IFUNC

static __typeof (fork) *
__attribute__ ((used))
fork_resolve (void)
{
  return &__libc_fork;
}

#  ifdef HAVE_ASM_SET_DIRECTIVE
#   define DEFINE_FORK(name) \
  asm (".set " #name ", fork_resolve\n" \
       ".globl " #name "\n" \
       ".type " #name ", %gnu_indirect_function");
#  else
#   define DEFINE_FORK(name) \
  asm (#name " = fork_resolve\n" \
       ".globl " #name "\n" \
       ".type " #name ", %gnu_indirect_function");
#  endif

# else  /* !HAVE_IFUNC */

static pid_t __attribute__ ((used))
fork_compat (void)
{
  return __libc_fork ();
}

# define DEFINE_FORK(name) strong_alias (fork_compat, name)

# endif  /* HAVE_IFUNC */

DEFINE_FORK (fork_ifunc)
compat_symbol (libpthread, fork_ifunc, fork, GLIBC_2_0);

DEFINE_FORK (__fork_ifunc)
compat_symbol (libpthread, __fork_ifunc, __fork, GLIBC_2_0);

#endif
