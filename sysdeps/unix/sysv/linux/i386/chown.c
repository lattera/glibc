/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

/*
  In Linux 2.1.x the chown functions have been changed.  A new function lchown
  was introduced.  The new chown now follows symlinks - the old chown and the
  new lchown do not follow symlinks.
  The new lchown function has the same number as the old chown had and the
  new chown has a new number.  When compiling with headers from Linux > 2.1.8x
  it's impossible to run this libc with older kernels.  In these cases libc
  has therefore to route calls to chown to the old chown function.
*/

extern int __syscall_chown (const char *__file,
			    uid_t __owner, gid_t __group);
#ifdef __NR_lchown
/* Running under Linux > 2.1.80.  */
static int __libc_old_chown;


int
__real_chown (const char *file, uid_t owner, gid_t group)
{
  int result;

  if (!__libc_old_chown)
    {
      int saved_errno = errno;
      result = INLINE_SYSCALL (chown, 3, file, owner, group);

      if (result >= 0 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __libc_old_chown = 1;
    }

  return __lchown (file, owner, group);
}
#endif


#ifndef __NR_lchown
/* Compiling under older kernels.  */
int
__chown_is_lchown (const char *file, uid_t owner, gid_t group)
{
  return INLINE_SYSCALL (chown, 3, file, owner, group);
}
#elif defined HAVE_ELF && defined PIC && defined DO_VERSIONING
/* Compiling for compatibiity.  */
int
__chown_is_lchown (const char *file, uid_t owner, gid_t group)
{
  return __lchown (file, owner, group);
}
#endif

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
strong_alias (__chown_is_lchown, _chown_is_lchown)
symbol_version (__chown_is_lchown, __chown, GLIBC_2.0);
symbol_version (_chown_is_lchown, chown, GLIBC_2.0);

# ifdef __NR_lchown
strong_alias (__real_chown, _real_chown)
default_symbol_version (__real_chown, __chown, GLIBC_2.1);
default_symbol_version (_real_chown, chown, GLIBC_2.1);
# else
strong_alias (__chown_is_lchown, __chown_is_lchown21)
strong_alias (__chown_is_lchown, _chown_is_lchown21)
default_symbol_version (__chown_is_lchown21, __chown, GLIBC_2.1);
default_symbol_version (_chown_is_lchown21, chown, GLIBC_2.1);
# endif
#else
# ifdef __NR_lchown
strong_alias (__real_chown, __chown)
weak_alias (__real_chown, chown)
# else
strong_alias (__chown_is_lchown, __chown)
weak_alias (__chown_is_lchown, chown)
# endif
#endif
