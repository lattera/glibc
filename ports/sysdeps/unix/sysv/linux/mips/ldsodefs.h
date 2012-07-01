/* Run-time dynamic linker data structures for loaded ELF shared objects. MIPS.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
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

#ifndef	_LDSODEFS_H

/* Get the real definitions.  */
#include_next <ldsodefs.h>

/* Now define our stuff.  */

/* We need special support to initialize DSO loaded for statically linked
   binaries.  */
extern void _dl_static_init (struct link_map *map);
#undef DL_STATIC_INIT
#define DL_STATIC_INIT(map) _dl_static_init (map)

/* Allow ABIVERSION == 1, meaning PLTs and copy relocations are
   required, with ELFOSABI_SYSV.  */
#undef VALID_ELF_ABIVERSION
#define VALID_ELF_ABIVERSION(osabi,ver)			\
  (ver == 0						\
   || (osabi == ELFOSABI_SYSV && ver < 2)		\
   || (osabi == ELFOSABI_LINUX && ver < LIBC_ABI_MAX))

#endif /* ldsodefs.h */
