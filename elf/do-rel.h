/* Do relocations for ELF dynamic linking.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

/* This file may be included twice, to define both
   `elf_dynamic_do_rel' and `elf_dynamic_do_rela'.  */

#ifdef DO_RELA
#define elf_dynamic_do_rel	elf_dynamic_do_rela
#define	Elf32_Rel		Elf32_Rela
#define elf_machine_rel		elf_machine_rela
#endif


/* Perform the relocations in MAP on the running program image as specified
   by RELTAG, SZTAG.  *RESOLVE is called to resolve symbol values; it
   modifies its argument pointer to point to the defining symbol, and
   returns the base load address of the defining object.  If LAZY is
   nonzero, this is the first pass on PLT relocations; they should be set
   up to call _dl_runtime_resolve, rather than fully resolved now.  */

static inline void
elf_dynamic_do_rel (struct link_map *map,
		    int reltag, int sztag,
		    Elf32_Addr (*resolve) (const Elf32_Sym **symbol,
					   Elf32_Addr r_offset),
		    int lazy)
{
  const Elf32_Sym *const symtab
    = (const Elf32_Sym *) (map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
  const Elf32_Rel *r
    = (const Elf32_Rel *) (map->l_addr + map->l_info[reltag]->d_un.d_ptr);
  const Elf32_Rel *end = &r[map->l_info[sztag]->d_un.d_val / sizeof *r];

  if (lazy)
    /* Doing lazy PLT relocations; they need very little info.  */
    for (; r < end; ++r)
      elf_machine_lazy_rel (map, r);
  else
    for (; r < end; ++r)
      {
	const Elf32_Sym *definer = &symtab[ELF32_R_SYM (r->r_info)];
	Elf32_Addr loadbase;

	if (ELF32_R_SYM (r->r_info) == STN_UNDEF)
	  loadbase = 0;		/* This value will not be consulted.  */
	else if (ELF32_ST_BIND (definer->st_info) == STB_LOCAL)
	  /* Local symbols always refer to the containing object.  */
	  loadbase = map->l_addr;
	else
	  {
	    if (resolve)
	      loadbase = (*resolve) (&definer, r->r_offset);
	    else
	      {
		assert (definer->st_shndx != SHN_UNDEF);
		loadbase = map->l_addr;
	      }
	  }
	elf_machine_rel (map, r, loadbase, definer);
      }
}

#undef elf_dynamic_do_rel
#undef Elf32_Rel
#undef elf_machine_rel
