/* Run-time dynamic linker data structures for loaded ELF shared objects.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#ifndef	_LDSODEFS_H

/* Get the real definitions.  */
#include_next <ldsodefs.h>

/* Now define our stuff.  */

/* Locating the auxiliary vector.  */
#ifndef DL_FIND_AUXV
# define DL_FIND_AUXV(auxp, envp) \
  do { \
    void **_tmp; \
    for (_tmp = (void **) (envp); *_tmp; ++_tmp) \
      continue; \
    (auxp) = (void *) ++_tmp; \
  } while (0)
#endif

#endif /* ldsodefs.h */
