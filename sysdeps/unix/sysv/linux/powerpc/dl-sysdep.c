/* Operating system support for run-time dynamic linker.  Linux/PPC version.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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


/* The PowerPC's auxiliary argument block gets aligned to a 16-byte
   boundary.  This is history and impossible to change compatibly.  */

#define DL_FIND_ARG_COMPONENTS(cookie, argc, argv, envp, auxp)	\
  do {								\
    char **_tmp;						\
    (argc) = *(long *) cookie;					\
    (argv) = (char **) cookie + 1;				\
    (envp) = (argv) + (argc) + 1;				\
    for (_tmp = (envp); *_tmp; ++_tmp)				\
      continue;							\
    /* The following '++' is important!  */			\
    ++_tmp;							\
    if (*_tmp == 0)						\
      {								\
	size_t _test = (size_t)_tmp;				\
	_test = _test + 0xf & ~0xf;				\
	/* When ld.so is being run directly, there is no	\
	   alignment (and no argument vector), so we make a	\
	   basic sanity check of the argument vector.  Of	\
	   course, this means that in future, the argument	\
	   vector will have to be laid out to allow for this	\
	   test :-(.  */					\
	if (((ElfW(auxv_t) *)_test)->a_type <= AT_PHDR)		\
	  _tmp = (char **)_test;				\
      }								\
    (auxp) = (ElfW(auxv_t) *) _tmp;				\
  } while (0)


#include <sysdeps/unix/sysv/linux/dl-sysdep.c>
