/* System dependent definitions for run-time dynamic loading.
   Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
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

#ifndef _DLFCN_H
# error "Never use <bits/dlfcn.h> directly; include <dlfcn.h> instead."
#endif

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x001	/* Lazy function call binding.  */
#define RTLD_NOW	0x002	/* Immediate function call binding.  */
#define	RTLD_BINDING_MASK 0x3	/* Mask of binding time value.  */

/* If the following bit is set in the MODE argument to `dlopen',
   the symbols of the loaded object and its dependencies are made
   visible as if the object were linked directly into the program.  */
#define RTLD_GLOBAL	0x004

__BEGIN_DECLS

/* Some SGI specific calls that aren't implemented yet.  */
extern void *sgidladd __P ((__const char *, int));
extern void *sgidlopen_version __P ((__const char *, int, __const char *,
				     int));
extern char *sgigetdsoversion __P ((__const char *));

__END_DECLS
