/* ELF symbol resolve functions for VDSO objects.
   Copyright (C) 2005, 2007 Free Software Foundation, Inc.
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

#ifndef _DL_VDSO_H
#define _DL_VDSO_H	1

#include <assert.h>
#include <ldsodefs.h>

#ifdef NDEBUG
# define CHECK_HASH(var) do {} while (0)
#else
# include <dl-hash.h>
# define CHECK_HASH(var) assert (var.hash == _dl_elf_hash (var.name))
#endif

/* Create version number record for lookup.  */
#define PREPARE_VERSION(var, vname, vhash) \
  struct r_found_version var;						      \
  var.name = vname;							      \
  var.hidden = 1;							      \
  var.hash = vhash;							      \
  CHECK_HASH (var);							      \
  /* We don't have a specific file where the symbol can be found.  */	      \
  var.filename = NULL


/* Functions for resolving symbols in the VDSO link map.  */
extern void *_dl_vdso_vsym (const char *name,
			    const struct r_found_version *version)
     internal_function attribute_hidden;

#endif /* dl-vdso.h */
