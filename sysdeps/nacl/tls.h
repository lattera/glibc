/* Definition for thread-local data handling.  NaCl version.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _NACL_TLS_H
#define _NACL_TLS_H	1

#ifndef __ASSEMBLER__

# include <nacl-interfaces.h>

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp) \
  ((*__nacl_irt_tls.tls_init) (tcbp) == 0 ? NULL : "tls_init call failed")

/* Our use of dl_sysinfo is rather different from the Linux syscall
   entry-point case.  We never need a thread-local copy of the value.  */
# undef SETUP_THREAD_SYSINFO
# undef CHECK_THREAD_SYSINFO
# define SETUP_THREAD_SYSINFO(pd) ((void) (pd))
# define CHECK_THREAD_SYSINFO(pd) ((void) (pd))

#endif /* __ASSEMBLER__ */

#endif  /* tls.h */
