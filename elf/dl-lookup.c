/* Look up a symbol in the loaded objects.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stddef.h>
#include <link.h>
#include <assert.h>
#include <string.h>


/* This is the hashing function specified by the ELF ABI.  */
static inline unsigned
_dl_elf_hash (const char *name)
{
  unsigned long int hash = 0;
  while (*name != '\0')
    {
      unsigned long int hi;
      hash = (hash << 4) + *name++;
      hi = hash & 0xf0000000;
      if (hi != 0)
	{
	  hash ^= hi >> 24;
	  /* The ELF ABI says `hash &= ~hi', but this is equivalent
	     in this case and on some machines one insn instead of two.  */
	  hash ^= hi;
	}
    }
  return hash;
}

/* Search loaded objects' symbol tables for a definition of the symbol
   UNDEF_NAME.  FLAGS is a set of flags.  If DL_LOOKUP_NOEXEC is set,
   then don't search the executable for a definition; this used for
   copy relocs.  If DL_LOOKUP_NOPLT is set, then a PLT entry cannot
   satisfy the reference; some different binding must be found.  */

ElfW(Addr)
_dl_lookup_symbol (const char *undef_name, const ElfW(Sym) **ref,
		   struct link_map *symbol_scope[],
		   const char *reference_name,
		   int flags)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct
    {
      ElfW(Addr) a;
      const ElfW(Sym) *s;
    } weak_value = { 0, NULL };
  size_t i;
  struct link_map **scope, *map;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    for (i = 0; i < (*scope)->l_nsearchlist; ++i)
      {
	const ElfW(Sym) *symtab;
	const char *strtab;
	ElfW(Symndx) symidx;

	map = (*scope)->l_searchlist[i];

	/* Don't search the executable when resolving a copy reloc.  */
	if (flags & DL_LOOKUP_NOEXEC && map->l_type == lt_executable)
	  continue;

	symtab = ((void *) map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
	strtab = ((void *) map->l_addr + map->l_info[DT_STRTAB]->d_un.d_ptr);

	/* Search the appropriate hash bucket in this object's symbol table
	   for a definition for the same symbol name.  */
	for (symidx = map->l_buckets[hash % map->l_nbuckets];
	     symidx != STN_UNDEF;
	     symidx = map->l_chain[symidx])
	  {
	    const ElfW(Sym) *sym = &symtab[symidx];

	    if (sym->st_value == 0 || /* No value.  */
		((flags & DL_LOOKUP_NOPLT) != 0 /* Reject PLT entry.  */
		 && sym->st_shndx == SHN_UNDEF))
	      continue;

	    switch (ELFW(ST_TYPE) (sym->st_info))
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

	    switch (ELFW(ST_BIND) (sym->st_info))
	      {
	      case STB_GLOBAL:
		/* Global definition.  Just what we need.  */
		*ref = sym;
		return map->l_addr;
	      case STB_WEAK:
		/* Weak definition.  Use this value if we don't find
		   another.  */
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

  if (weak_value.s == NULL &&
      (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK))
    {
      /* We could find no value for a strong reference.  */
      const char msg[] = "undefined symbol: ";
      const size_t len = strlen (undef_name);
      char buf[sizeof msg + len];
      memcpy (buf, msg, sizeof msg - 1);
      memcpy (&buf[sizeof msg - 1], undef_name, len + 1);
      _dl_signal_error (0, reference_name, buf);
    }

  *ref = weak_value.s;
  return weak_value.a;
}


/* Cache the location of MAP's hash table.  */

void
_dl_setup_hash (struct link_map *map)
{
  ElfW(Symndx) *hash = (void *)(map->l_addr + map->l_info[DT_HASH]->d_un.d_ptr);
  ElfW(Symndx) nchain;
  map->l_nbuckets = *hash++;
  nchain = *hash++;
  map->l_buckets = hash;
  hash += map->l_nbuckets;
  map->l_chain = hash;
}
