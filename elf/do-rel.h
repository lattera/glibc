/* Do relocations for ELF dynamic linking.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

/* This file may be included twice, to define both
   `elf_dynamic_do_rel' and `elf_dynamic_do_rela'.  */

#ifdef DO_RELA
# define elf_dynamic_do_rel	elf_dynamic_do_rela
# define Rel			Rela
# define elf_machine_rel	elf_machine_rela
#endif

#ifndef VERSYMIDX
# define VERSYMIDX(sym)	(DT_NUM + DT_PROCNUM + DT_VERSIONTAGIDX (sym))
#endif

/* Perform the relocations in MAP on the running program image as specified
   by RELTAG, SZTAG.  If LAZY is nonzero, this is the first pass on PLT
   relocations; they should be set up to call _dl_runtime_resolve, rather
   than fully resolved now.  */

static inline void
elf_dynamic_do_rel (struct link_map *map,
		    ElfW(Addr) reladdr, ElfW(Addr) relsize,
		    int lazy)
{
  const ElfW(Rel) *r = (const ElfW(Rel) *)(map->l_addr + reladdr);
  const ElfW(Rel) *end = (const ElfW(Rel) *)(map->l_addr + reladdr + relsize);

  if (lazy)
    {
      /* Doing lazy PLT relocations; they need very little info.  */
      ElfW(Addr) l_addr = map->l_addr;
      for (; r < end; ++r)
	elf_machine_lazy_rel (l_addr, r);
    }
  else
    {
      const ElfW(Sym) *const symtab =
	(const ElfW(Sym) *) (map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);

      if (map->l_info[VERSYMIDX (DT_VERSYM)])
	{
	  const ElfW(Half) *const version =
	    (const ElfW(Half) *) (map->l_addr
				  + map->l_info[VERSYMIDX (DT_VERSYM)]->d_un.d_ptr);

	  for (; r < end; ++r)
	    {
	      ElfW(Half) ndx = version[ELFW(R_SYM) (r->r_info)];
	      elf_machine_rel (map, r, &symtab[ELFW(R_SYM) (r->r_info)],
			       &map->l_versions[ndx],
			       (void *) (map->l_addr + r->r_offset));
	    }
	}
      else
	for (; r < end; ++r)
	  elf_machine_rel (map, r, &symtab[ELFW(R_SYM) (r->r_info)], NULL,
			   (void *) (map->l_addr + r->r_offset));
    }
}

#undef elf_dynamic_do_rel
#undef Rel
#undef elf_machine_rel
