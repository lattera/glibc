/* Locate the shared object symbol nearest a given address.
   Copyright (C) 1996-2003, 2004   Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <stddef.h>
#include <ldsodefs.h>


int
internal_function
_dl_addr (const void *address, Dl_info *info,
	  struct link_map **mapp, const ElfW(Sym) **symbolp)
{
  const ElfW(Addr) addr = DL_LOOKUP_ADDRESS (address);
  struct link_map *match;
  const ElfW(Sym) *symtab, *matchsym, *symtabend;
  const char *strtab;
  ElfW(Word) strtabsize;

  /* Protect against concurrent loads and unloads.  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));

  /* Find the highest-addressed object that ADDRESS is not below.  */
  match = NULL;
  for (Lmid_t ns = 0; ns < DL_NNS; ++ns)
    for (struct link_map *l = GL(dl_ns)[ns]._ns_loaded; l; l = l->l_next)
      if (addr >= l->l_map_start && addr < l->l_map_end)
	{
	  /* We know ADDRESS lies within L if in any shared object.
	     Make sure it isn't past the end of L's segments.  */
	  size_t n = l->l_phnum;
	  if (n > 0)
	    {
	      do
		--n;
	      while (l->l_phdr[n].p_type != PT_LOAD);
	      if (addr >= (l->l_addr +
			   l->l_phdr[n].p_vaddr + l->l_phdr[n].p_memsz))
		/* Off the end of the highest-addressed shared object.  */
		continue;
	    }

	  match = l;
	  break;
	}

  int result = 0;
  if (match != NULL)
    {
      /* Now we know what object the address lies in.  */
      info->dli_fname = match->l_name;
      info->dli_fbase = (void *) match->l_map_start;

      /* If this is the main program the information is incomplete.  */
      if (__builtin_expect (match->l_name[0], 'a') == '\0'
	  && match->l_type == lt_executable)
	info->dli_fname = _dl_argv[0];

      symtab = (const void *) D_PTR (match, l_info[DT_SYMTAB]);
      strtab = (const void *) D_PTR (match, l_info[DT_STRTAB]);

      strtabsize = match->l_info[DT_STRSZ]->d_un.d_val;

      if (match->l_info[DT_HASH] != NULL)
	symtabend = (symtab
		     + ((Elf_Symndx *) D_PTR (match, l_info[DT_HASH]))[1]);
      else
	/* There is no direct way to determine the number of symbols in the
	   dynamic symbol table and no hash table is present.  The ELF
	   binary is ill-formed but what shall we do?  Use the beginning of
	   the string table which generally follows the symbol table.  */
	symtabend = (const ElfW(Sym) *) strtab;

      /* We assume that the string table follows the symbol table,
	 because there is no way in ELF to know the size of the
	 dynamic symbol table!!  */
      for (matchsym = NULL; (void *) symtab < (void *) symtabend; ++symtab)
	if (addr >= match->l_addr + symtab->st_value
#if defined USE_TLS
	    && ELFW(ST_TYPE) (symtab->st_info) != STT_TLS
#endif
	    && ((symtab->st_size == 0
		 && addr == match->l_addr + symtab->st_value)
		|| addr < match->l_addr + symtab->st_value + symtab->st_size)
	    && symtab->st_name < strtabsize
	    && (matchsym == NULL || matchsym->st_value < symtab->st_value)
	    && (ELFW(ST_BIND) (symtab->st_info) == STB_GLOBAL
		|| ELFW(ST_BIND) (symtab->st_info) == STB_WEAK))
	  matchsym = (ElfW(Sym) *) symtab;

      if (mapp)
	*mapp = match;
      if (symbolp)
	*symbolp = matchsym;

      if (matchsym)
	{
	  /* We found a symbol close by.  Fill in its name and exact
	     address.  */
	  lookup_t matchl = LOOKUP_VALUE (match);

	  info->dli_sname = strtab + matchsym->st_name;
	  info->dli_saddr = DL_SYMBOL_ADDRESS (matchl, matchsym);
	}
      else
	{
	  /* No symbol matches.  We return only the containing object.  */
	  info->dli_sname = NULL;
	  info->dli_saddr = NULL;
	}

      result = 1;
    }

  __rtld_lock_unlock_recursive (GL(dl_load_lock));

  return result;
}
libc_hidden_def (_dl_addr)
