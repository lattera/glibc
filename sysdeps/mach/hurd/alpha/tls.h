/* Definition for thread-local data handling.  Hurd/Alpha version.
   Copyright (C) 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _TLS_H
#define _TLS_H

#if defined HAVE_TLS_SUPPORT && 0

/* Signal that TLS support is available.  */
# define USE_TLS	1

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(descr)						      \
  do									      \
  {									      \
    register tcbhead_t *_a0 __asm__ ("$16") = (descr);			      \
    __asm__ ("call_pal %0" : : "i" (PAL_wruniq), "r" (_a0));		      \
  } while (0)

# define THREAD_TCB()							      \
  ({									      \
    register tcbhead_t *_rv __asm__ ("$0");				      \
    __asm__ ("call_pal %0" : "=r" (rv) : "i" (PAL_rduniq));		      \
    _rv;								      \
  })

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv)		(THREAD_DTV () = (dtv))

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() 			(THREAD_TCB ()->dtv)

#endif /* HAVE_TLS_SUPPORT */

#endif	/* tls.h */
