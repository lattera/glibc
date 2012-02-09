/* Operating system support for run-time dynamic linker.  Linux/PPC version.
   Copyright (C) 1997, 1998, 2001, 2003, 2006, 2007
   Free Software Foundation, Inc.
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

#include <config.h>
#include <kernel-features.h>
#include <ldsodefs.h>

int __cache_line_size attribute_hidden;

/* Scan the Aux Vector for the "Data Cache Block Size" entry.  If found
   verify that the static extern __cache_line_size is defined by checking
   for not NULL.  If it is defined then assign the cache block size
   value to __cache_line_size.  */
#define DL_PLATFORM_AUXV						      \
      case AT_DCACHEBSIZE:						      \
	__cache_line_size = av->a_un.a_val;				      \
	break;

#ifndef __ASSUME_STD_AUXV

/* The PowerPC's auxiliary argument block gets aligned to a 16-byte
   boundary.  This is history and impossible to change compatibly.  */

#define DL_FIND_ARG_COMPONENTS(cookie, argc, argv, envp, auxp) \
  do {									      \
    char **_tmp;							      \
    size_t _test;							      \
    (argc) = *(long int *) cookie;					      \
    (argv) = (char **) cookie + 1;					      \
    (envp) = (argv) + (argc) + 1;					      \
    for (_tmp = (envp); *_tmp; ++_tmp)					      \
      continue;								      \
    /* The following '++' is important!  */				      \
    ++_tmp;								      \
									      \
    _test = (size_t)_tmp;						      \
    _test = (_test + 0xf) & ~0xf;					      \
    /* Under some circumstances, MkLinux (up to at least DR3a5)		      \
       omits the padding.  To work around this, we make a		      \
       basic sanity check of the argument vector.  Of			      \
       course, this means that in future, the argument			      \
       vector will have to be laid out to allow for this		      \
       test :-(.  */							      \
     if (((ElfW(auxv_t) *)_test)->a_type <= 0x10)			      \
       _tmp = (char **)_test;						      \
    (auxp) = (ElfW(auxv_t) *) _tmp;					      \
  } while (0)
#endif

#include <sysdeps/unix/sysv/linux/dl-sysdep.c>
