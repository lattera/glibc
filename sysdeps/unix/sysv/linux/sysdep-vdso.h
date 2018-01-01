/* vDSO common definition for Linux.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

#ifndef SYSDEP_VDSO_LINUX_H
# define SYSDEP_VDSO_LINUX_H

#define VDSO_SYMBOL(__name) __vdso_##__name

#ifndef INTERNAL_VSYSCALL_CALL
# define INTERNAL_VSYSCALL_CALL(funcptr, err, nr, args...)		      \
     funcptr (args)
#endif

#ifdef SHARED

# ifdef HAVE_VSYSCALL

#  include <libc-vdso.h>

#  define INLINE_VSYSCALL(name, nr, args...)				      \
  ({									      \
    __label__ out;							      \
    __label__ iserr;							      \
    INTERNAL_SYSCALL_DECL (sc_err);					      \
    long int sc_ret;							      \
									      \
    __typeof (__vdso_##name) vdsop = __vdso_##name;			      \
    PTR_DEMANGLE (vdsop);						      \
    if (vdsop != NULL)							      \
      {									      \
	sc_ret = INTERNAL_VSYSCALL_CALL (vdsop, sc_err, nr, ##args);	      \
	if (!INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			      \
	  goto out;							      \
	if (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err) != ENOSYS)		      \
	  goto iserr;							      \
      }									      \
									      \
    sc_ret = INTERNAL_SYSCALL (name, sc_err, nr, ##args);		      \
    if (INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			      \
      {									      \
      iserr:								      \
        __set_errno (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err));		      \
        sc_ret = -1L;							      \
      }									      \
  out:									      \
    sc_ret;								      \
  })

#  define INTERNAL_VSYSCALL(name, err, nr, args...)			      \
  ({									      \
    __label__ out;							      \
    long v_ret;								      \
									      \
    __typeof (__vdso_##name) vdsop = __vdso_##name;			      \
    PTR_DEMANGLE (vdsop);						      \
    if (vdsop != NULL)							      \
      {									      \
	v_ret = INTERNAL_VSYSCALL_CALL (vdsop, err, nr, ##args);	      \
	if (!INTERNAL_SYSCALL_ERROR_P (v_ret, err)			      \
	    || INTERNAL_SYSCALL_ERRNO (v_ret, err) != ENOSYS)		      \
	  goto out;							      \
      }									      \
    v_ret = INTERNAL_SYSCALL (name, err, nr, ##args);			      \
  out:									      \
    v_ret;								      \
  })
# else
#  define INLINE_VSYSCALL(name, nr, args...) \
    INLINE_SYSCALL (name, nr, ##args)
#  define INTERNAL_VSYSCALL(name, err, nr, args...) \
    INTERNAL_SYSCALL (name, err, nr, ##args)
# endif /* HAVE_VSYSCALL  */

# else /* SHARED  */

#  define INLINE_VSYSCALL(name, nr, args...) \
    INLINE_SYSCALL (name, nr, ##args)
#  define INTERNAL_VSYSCALL(name, err, nr, args...) \
    INTERNAL_SYSCALL (name, err, nr, ##args)

#endif /* SHARED  */

#endif /* SYSDEP_VDSO_LINUX_H  */
