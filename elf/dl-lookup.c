/* Look up a symbol in the loaded objects.
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

#include <stddef.h>
#include <libelf.h>
#include <link.h>
#include <assert.h>

/* Search loaded objects' symbol tables for a definition of 
   the symbol UNDEF_NAME.  If NOSELF is nonzero, then *REF
   cannot satisfy the reference itself; some different binding
   must be found.  */

Elf32_Addr
_dl_lookup_symbol (const char *undef_name, const Elf32_Sym **ref,
		   struct link_map *symbol_scope,
		   const char *reference_name,
		   int noself)
{
  unsigned long int hash = elf_hash (undef_name);
  struct link_map *map;
  struct
    {
      Elf32_Addr a;
      const Elf32_Sym *s;
    } weak_value = { 0, NULL };

  /* Search the relevant loaded objects for a definition.  */
  for (map = symbol_scope; map; map = map->l_next)
    {
      const Elf32_Sym *symtab;
      const char *strtab;
      Elf32_Word symidx;

      symtab = ((void *) map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
      strtab = ((void *) map->l_addr + map->l_info[DT_STRTAB]->d_un.d_ptr);

      /* Search the appropriate hash bucket in this object's symbol table
	 for a definition for the same symbol name.  */
      for (symidx = map->l_buckets[hash % map->l_nbuckets];
	   symidx != STN_UNDEF;
	   symidx = map->l_chain[symidx])
	{
	  const Elf32_Sym *sym = &symtab[symidx];

	  if (sym->st_value == 0 || /* No value.  */
	      sym->st_shndx == SHN_UNDEF || /* PLT entry.  */
	      (noself && sym == *ref))	/* The reference can't define it.  */
	    continue;

	  switch (ELF32_ST_TYPE (sym->st_info))
	    {
	    case STT_NOTYPE:
	    case STT_FUNC:
	    case STT_OBJECT:
	      break;
	    default:
	      /* Not a code/data definition.  */
	      continue;
	    }

	  if (sym != *ref && strcmp (strtab + sym->st_name, undef_name))
	    /* Not the symbol we are looking for.  */
	    continue;

	  switch (ELF32_ST_BIND (sym->st_info))
	    {
	    case STB_GLOBAL:
	      /* Global definition.  Just what we need.  */
	      *ref = sym;
	      return map->l_addr;
	    case STB_WEAK:
	      /* Weak definition.  Use this value if we don't find another.  */
	      if (! weak_value.s)
		{
		  weak_value.s = sym;
		  weak_value.a = map->l_addr;
		}
	      break;
	    default:
	      /* Local symbols are ignored.  */
	      break;
	    }
	}
    }

  if (weak_value.s == NULL)
    {
      const char msg[] = "undefined symbol: ";
      char buf[sizeof msg + strlen (undef_name)];
      memcpy (buf, msg, sizeof msg - 1);
      memcpy (&buf[sizeof msg - 1], undef_name, sizeof buf - sizeof msg + 1);
      _dl_signal_error (0, reference_name, buf);
    }

  *ref = weak_value.s;
  return weak_value.a;
}


/* Cache the location of MAP's hash table.  */

void
_dl_setup_hash (struct link_map *map)
{
  Elf32_Word *hash = (void *) map->l_addr + map->l_info[DT_HASH]->d_un.d_ptr;
  Elf32_Word nchain;
  map->l_nbuckets = *hash++;
  nchain = *hash++;
  map->l_buckets = hash;
  hash += map->l_nbuckets;
  map->l_chain = hash;
}
