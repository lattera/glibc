/* Do relocations for ELF dynamic linking.
   Copyright (C) 1995,96,97,98,99,2000,2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* This file may be included twice, to define both
   `elf_dynamic_do_rel' and `elf_dynamic_do_rela'.  */

#ifdef DO_RELA
# define elf_dynamic_do_rel	elf_dynamic_do_rela
# define RELCOUNT_IDX		VERSYMIDX (DT_RELACOUNT)
# define Rel			Rela
# define elf_machine_rel	elf_machine_rela
#else
# define RELCOUNT_IDX		VERSYMIDX (DT_RELCOUNT)
#endif

#ifndef VERSYMIDX
# define VERSYMIDX(sym)	(DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (sym))
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
  const ElfW(Rel) *r = (const void *) reladdr;
  const ElfW(Rel) *end = (const void *) (reladdr + relsize);
  ElfW(Addr) l_addr = map->l_addr;

  if (lazy)
    {
      /* Doing lazy PLT relocations; they need very little info.  */
      for (; r < end; ++r)
	elf_machine_lazy_rel (map, l_addr, r);
    }
  else
    {
      const ElfW(Sym) *const symtab =
	(const void *) D_PTR (map, l_info[DT_SYMTAB]);
      ElfW(Word) nrelative = (map->l_info[RELCOUNT_IDX] == NULL
			      ? 0 : map->l_info[RELCOUNT_IDX]->d_un.d_val);
      const ElfW(Rel) *endrel = end;
      end -= nrelative;

      if (map->l_info[VERSYMIDX (DT_VERSYM)])
	{
	  const ElfW(Half) *const version =
	    (const void *) D_PTR (map, l_info[VERSYMIDX (DT_VERSYM)]);

	  for (; r < end; ++r)
	    {
	      ElfW(Half) ndx = version[ELFW(R_SYM) (r->r_info)];
	      elf_machine_rel (map, r, &symtab[ELFW(R_SYM) (r->r_info)],
			       &map->l_versions[ndx],
			       (void *) (l_addr + r->r_offset));
	    }
	}
      else
	for (; r < end; ++r)
	  elf_machine_rel (map, r, &symtab[ELFW(R_SYM) (r->r_info)], NULL,
			   (void *) (l_addr + r->r_offset));

#ifndef RTLD_BOOTSTRAP
      /* This is defined in rtld.c, but nowhere in the static libc.a; make
	 the reference weak so static programs can still link.  This
	 declaration cannot be done when compiling rtld.c (i.e. #ifdef
	 RTLD_BOOTSTRAP) because rtld.c contains the common defn for
	 _dl_rtld_map, which is incompatible with a weak decl in the same
	 file.  */
      weak_extern (_dl_rtld_map);
      if (map != &_dl_rtld_map) /* Already done in rtld itself.  */
#endif
	for (; r < endrel; ++r)
	  elf_machine_rel_relative (l_addr, r,
				    (void *) (l_addr + r->r_offset));
    }
}

#undef elf_dynamic_do_rel
#undef Rel
#undef elf_machine_rel
