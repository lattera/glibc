/* errnos.h - error constants.  Linux specific version.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifdef _ERRNO_H

# undef EDOM
# undef ERANGE
# include <linux/errno.h>

# ifndef __ASSEMBLER__
#  ifdef _LIBC
/* We now need a declaration of the `errno' variable.  */
extern int errno;

/* Function to get address of global `errno' variable.  */
extern int *__errno_location __P ((void)) __attribute__ ((__const__));

#   ifdef _LIBC_REENTRANT
static inline int
__set_errno (int __err)
{
  return *__errno_location () = errno = __err;
}
#   else /* !_LIBC_REENTRANT */
#    define __set_errno(val) errno = (val)
#   endif /* _LIBC_REENTRANT */
#  endif /* _LIBC */

#  if defined __USE_REENTRANT && (!defined _LIBC || defined _LIBC_REENTRANT)
/* When using threads, errno is a per-thread value.  */
#   define errno (*__errno_location ())
#  endif
# endif /* !__ASSEMBLER__ */
#endif /* _ERRNO_H */

#if !defined (_ERRNO_H) && defined (__need_Emath)
/* This is ugly but the kernel header is not clean enough.  We must
   define only the values EDOM and ERANGE in case __need_Emath is
   defined.  The value is the same for all Linux ports.  */
# define EDOM	33	/* Math argument out of domain of function.  */
# define ERANGE	34	/* Math result not representable.  */
#endif /* !_ERRNO_H && __need_Emath */
