/* AIX dependent definitions for run-time dynamic loading.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#ifndef _DLFCN_H
# error "Never use <bits/dlfcn.h> directly; include <dlfcn.h> instead."
#endif

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x004	/* Lazy function call binding.  */
#define RTLD_NOW	0x002	/* Immediate function call binding.  */
#define	RTLD_BINDING_MASK 0x6	/* Mask of binding time value.  */

/* If the following bit is set in the MODE argument to `dlopen',
   the symbols of the loaded object and its dependencies are made
   visible as if the object were linked directly into the program.  */
#define RTLD_GLOBAL	0x10000

/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
   The implementation does this by default and so we can define the
   value to zero.  */
#define RTLD_LOCAL	0x800000

#ifdef __USE_GNU
/* These are extensions of the AIX kernel.  */
# define RTLD_NOAUTODEFER	0x020000
# define RTLD_MEMBER		0x040000
# define RTLD_UNIX_LATEST	0x100000
#endif

/* No support to profile shared libraries available.  */
# define DL_CALL_FCT(fctp, args) fctp args
