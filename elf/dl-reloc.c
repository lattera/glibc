/* Relocate a shared object and resolve its references to other loaded objects.
Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <link.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "dynamic-link.h"


void
_dl_relocate_object (struct link_map *l, int lazy)
{
  const size_t pagesize = getpagesize ();

  if (l->l_relocated)
    return;

  if (l->l_info[DT_TEXTREL])
    {
      /* Bletch.  We must make read-only segments writable
	 long enough to relocate them.  */
      const Elf32_Phdr *ph;
      for (ph = l->l_phdr; ph < &l->l_phdr[l->l_phnum]; ++ph)
	if (ph->p_type == PT_LOAD && (ph->p_flags & PF_W) == 0)
	  {
	    caddr_t mapstart = ((caddr_t) l->l_addr +
				(ph->p_vaddr & ~(pagesize - 1)));
	    caddr_t mapend = ((caddr_t) l->l_addr +
			      ((ph->p_vaddr + ph->p_memsz + pagesize - 1)
			       & ~(pagesize - 1)));
	    if (mprotect (mapstart, mapend - mapstart,
			  PROT_READ|PROT_WRITE) < 0)
	      _dl_signal_error (errno, l->l_name,
				"cannot make segment writable for relocation");
	  }
    }

  {
    struct link_map *real_next, *scope;

    const char *strtab
      = ((void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);


    Elf32_Addr resolve (const Elf32_Sym **ref, Elf32_Addr r_offset)
      {
	return _dl_lookup_symbol (strtab + (*ref)->st_name, ref, scope,
				  l->l_name, (*ref)->st_value == r_offset);
      }

    real_next = l->l_next;
    if (l->l_info[DT_SYMBOLIC])
      {
	if (l->l_prev)
	  l->l_prev->l_next = real_next;
	l->l_next = _dl_loaded;
	scope = l;
      }
    else
      scope = _dl_loaded;

    if (l->l_type == lt_interpreter)
      /* We cannot be lazy when relocating the dynamic linker itself.  It
	 was previously relocated eagerly (allowing us to be running now),
	 and needs always to be fully relocated so it can run without the
	 aid of run-time fixups (because it's the one to do them), so we
	 must always re-relocate its PLT eagerly.  */
      lazy = 0;

    ELF_DYNAMIC_RELOCATE (l, lazy, resolve);

    /* Restore list frobnication done above for DT_SYMBOLIC.  */
    l->l_next = real_next;
    if (l->l_prev)
      l->l_prev->l_next = l;
  }

  if (l->l_info[DT_JMPREL] && lazy)
    /* Set up the PLT so its unrelocated entries will
       jump to _dl_runtime_resolve, which will relocate them.  */
    elf_machine_runtime_setup (l);

  l->l_relocated = 1;

  if (l->l_info[DT_TEXTREL])
    {
      /* Undo the protection change we made before relocating.  */
      const Elf32_Phdr *ph;
      for (ph = l->l_phdr; ph < &l->l_phdr[l->l_phnum]; ++ph)
	if (ph->p_type == PT_LOAD && (ph->p_flags & PF_W) == 0)
	  {
	    caddr_t mapstart = ((caddr_t) l->l_addr +
				(ph->p_vaddr & ~(pagesize - 1)));
	    caddr_t mapend = ((caddr_t) l->l_addr +
			      ((ph->p_vaddr + ph->p_memsz + pagesize - 1)
			       & ~(pagesize - 1)));
	    int prot = 0;
	    if (ph->p_flags & PF_R)
	      prot |= PROT_READ;
	    if (ph->p_flags & PF_X)
	      prot |= PROT_EXEC;
	    if (mprotect (mapstart, mapend - mapstart, prot) < 0)
	      _dl_signal_error (errno, l->l_name,
				"can't restore segment prot after reloc");
	  }
    }

}
