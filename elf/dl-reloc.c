/* Relocate a shared object and resolve its references to other loaded objects.
   Copyright (C) 1995,96,97,98,99,2000,2001 Free Software Foundation, Inc.
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

#include <errno.h>
#include <libintl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "dynamic-link.h"


void
_dl_relocate_object (struct link_map *l, struct r_scope_elem *scope[],
		     int lazy, int consider_profiling)
{
  if (l->l_relocated)
    return;

  /* If DT_BIND_NOW is set relocate all references in this object.  We
     do not do this if we are profiling, of course.  */
  if (!consider_profiling
      && __builtin_expect (l->l_info[DT_BIND_NOW] != NULL, 0))
    lazy = 0;

  if (__builtin_expect (_dl_debug_mask & DL_DEBUG_RELOC, 0))
    _dl_printf ("\nrelocation processing: %s%s\n",
		l->l_name[0] ? l->l_name : _dl_argv[0], lazy ? " (lazy)" : "");

  if (__builtin_expect (l->l_info[DT_TEXTREL] != NULL, 0))
    {
      /* Bletch.  We must make read-only segments writable
	 long enough to relocate them.  */
      const ElfW(Phdr) *ph;
      for (ph = l->l_phdr; ph < &l->l_phdr[l->l_phnum]; ++ph)
	if (ph->p_type == PT_LOAD && (ph->p_flags & PF_W) == 0)
	  {
	    caddr_t mapstart = ((caddr_t) l->l_addr +
				(ph->p_vaddr & ~(_dl_pagesize - 1)));
	    caddr_t mapend = ((caddr_t) l->l_addr +
			      ((ph->p_vaddr + ph->p_memsz + _dl_pagesize - 1)
			       & ~(_dl_pagesize - 1)));
	    if (__builtin_expect (__mprotect (mapstart, mapend - mapstart,
					      PROT_READ|PROT_WRITE), 0) < 0)
	      _dl_signal_error (errno, l->l_name, N_("\
cannot make segment writable for relocation"));
	  }
    }

  {
    /* Do the actual relocation of the object's GOT and other data.  */

    /* String table object symbols.  */
    const char *strtab = (const void *) D_PTR (l, l_info[DT_STRTAB]);

    /* This macro is used as a callback from the ELF_DYNAMIC_RELOCATE code.  */
#define RESOLVE_MAP(ref, version, flags) \
    (ELFW(ST_BIND) ((*ref)->st_info) != STB_LOCAL			      \
     ? ((version) != NULL && (version)->hash != 0			      \
	? _dl_lookup_versioned_symbol (strtab + (*ref)->st_name, l, (ref),    \
				       scope, (version), (flags), 0)	      \
	: _dl_lookup_symbol (strtab + (*ref)->st_name, l, (ref), scope,	      \
			     (flags), 0))				      \
     : l)
#define RESOLVE(ref, version, flags) \
    (ELFW(ST_BIND) ((*ref)->st_info) != STB_LOCAL			      \
     ? ((version) != NULL && (version)->hash != 0			      \
	? _dl_lookup_versioned_symbol (strtab + (*ref)->st_name, l, (ref),    \
				       scope, (version), (flags), 0)	      \
	: _dl_lookup_symbol (strtab + (*ref)->st_name, l, (ref), scope,	      \
			     (flags), 0))				      \
     : l->l_addr)

#include "dynamic-link.h"
    ELF_DYNAMIC_RELOCATE (l, lazy, consider_profiling);

    if (__builtin_expect (_dl_profile != NULL, 0))
      {
	/* Allocate the array which will contain the already found
	   relocations.  If the shared object lacks a PLT (for example
	   if it only contains lead function) the l_info[DT_PLTRELSZ]
	   will be NULL.  */
	if (l->l_info[DT_PLTRELSZ] == NULL)
	  _dl_fatal_printf ("%s: profiler found no PLTREL in object %s\n",
			    _dl_argv[0] ?: "<program name unknown>",
			    l->l_name);

	l->l_reloc_result =
	  (ElfW(Addr) *) calloc (sizeof (ElfW(Addr)),
				 l->l_info[DT_PLTRELSZ]->d_un.d_val);
	if (l->l_reloc_result == NULL)
	  _dl_fatal_printf ("\
%s: profiler out of memory shadowing PLTREL of %s\n",
			    _dl_argv[0] ?: "<program name unknown>",
			    l->l_name);
      }
  }

  /* Mark the object so we know this work has been done.  */
  l->l_relocated = 1;

  /* DT_TEXTREL is now in level 2 and might phase out at some time.
     But we rewrite the DT_FLAGS entry to make testing easier and
     therefore it will be available at all time.  */
  if (__builtin_expect (l->l_info[DT_TEXTREL] != NULL, 0))
    {
      /* Undo the protection change we made before relocating.  */
      const ElfW(Phdr) *ph;
      for (ph = l->l_phdr; ph < &l->l_phdr[l->l_phnum]; ++ph)
	if (ph->p_type == PT_LOAD && (ph->p_flags & PF_W) == 0)
	  {
	    caddr_t mapstart = ((caddr_t) l->l_addr +
				(ph->p_vaddr & ~(_dl_pagesize - 1)));
	    caddr_t mapend = ((caddr_t) l->l_addr +
			      ((ph->p_vaddr + ph->p_memsz + _dl_pagesize - 1)
			       & ~(_dl_pagesize - 1)));
	    extern unsigned char _dl_pf_to_prot[8];
	    int prot;

	    if ((PF_R | PF_W | PF_X) == 7
		&& (PROT_READ | PROT_WRITE | PROT_EXEC) == 7)
	      prot = _dl_pf_to_prot[ph->p_flags & (PF_R | PF_X)];
	    else
	      {
		prot = 0;
		if (ph->p_flags & PF_R)
		  prot |= PROT_READ;
		if (ph->p_flags & PF_X)
		  prot |= PROT_EXEC;
	      }

	    if (__builtin_expect (__mprotect (mapstart, mapend - mapstart,
					      prot), 0) < 0)
	      _dl_signal_error (errno, l->l_name,
				N_("can't restore segment prot after reloc"));

#ifdef CLEAR_CACHE
	    CLEAR_CACHE (mapstart, mapend);
#endif
	  }
    }
}

#include "../stdio-common/_itoa.h"
#define DIGIT(b)	_itoa_lower_digits[(b) & 0xf];

void
internal_function
_dl_reloc_bad_type (struct link_map *map, uint_fast8_t type, int plt)
{
  extern const char _itoa_lower_digits[];
  if (plt)
    {
      /* XXX We cannot translate the message.  */
      static char msg[] = "unexpected PLT reloc type 0x??";
      msg[sizeof msg - 3] = DIGIT(type >> 4);
      msg[sizeof msg - 2] = DIGIT(type);
      _dl_signal_error (0, map->l_name, msg);
    }
  else
    {
      /* XXX We cannot translate the message.  */
      static char msg[] = "unexpected reloc type 0x??";
      msg[sizeof msg - 3] = DIGIT(type >> 4);
      msg[sizeof msg - 2] = DIGIT(type);
      _dl_signal_error (0, map->l_name, msg);
    }
}
