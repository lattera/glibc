/* Resolve conflicts against already prelinked libraries.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

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

#include <errno.h>
#include <libintl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/types.h>
#include "dynamic-link.h"

extern unsigned long int _dl_num_cache_relocations;	/* in dl-lookup.c */

void
_dl_resolve_conflicts (struct link_map *l, ElfW(Rela) *conflict,
		       ElfW(Rela) *conflictend)
{
  if (__builtin_expect (_dl_debug_mask & DL_DEBUG_RELOC, 0))
    _dl_printf ("\nconflict processing: %s\n",
		l->l_name[0] ? l->l_name : _dl_argv[0]);

  {
    /* Do the conflict relocation of the object and library GOT and other
       data.  */

    /* This macro is used as a callback from the ELF_DYNAMIC_RELOCATE code.  */
#define RESOLVE_MAP(ref, version, flags) (*ref = NULL, 0)
#define RESOLVE(ref, version, flags) (*ref = NULL, 0)
#define RESOLVE_CONFLICT_FIND_MAP(map, r_offset)		\
do								\
  {								\
    while ((resolve_conflict_map->l_map_end			\
	    < (ElfW(Addr))(r_offset))				\
	   || (resolve_conflict_map->l_map_start		\
	       > (ElfW(Addr))(r_offset)))			\
      resolve_conflict_map					\
	= resolve_conflict_map->l_next;				\
    (map) = resolve_conflict_map;				\
  } while (0)

    struct link_map *resolve_conflict_map __attribute__ ((__unused__))
      = _dl_loaded;
    

#include "dynamic-link.h"

    _dl_num_cache_relocations += conflictend - conflict;

    for (; conflict < conflictend; ++conflict)
      elf_machine_rela (l, conflict, NULL, NULL, (void *) conflict->r_offset);
  }
}
