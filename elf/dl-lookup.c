/* Look up a symbol in the loaded objects.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include "../stdio-common/_itoa.h"

#define VERSTAG(tag)	(DT_NUM + DT_PROCNUM + DT_VERSIONTAGIDX (tag))

struct sym_val
  {
    ElfW(Addr) a;
    const ElfW(Sym) *s;
  };


/* This is the hashing function specified by the ELF ABI.  In the
   first five operations now overflow is possible so we optimized it a
   bit.  */
static inline unsigned
_dl_elf_hash (const char *name)
{
  unsigned long int hash = 0;
  if (*name != '\0')
    {
      hash = (hash << 4) + *name++;
      if (*name != '\0')
	{
	  hash = (hash << 4) + *name++;
	  if (*name != '\0')
	    {
	      hash = (hash << 4) + *name++;
	      if (*name != '\0')
		{
		  hash = (hash << 4) + *name++;
		  if (*name != '\0')
		    {
		      hash = (hash << 4) + *name++;
		      while (*name != '\0')
			{
			  unsigned long int hi;
			  hash = (hash << 4) + *name++;
			  hi = hash & 0xf0000000;
			  if (hi != 0)
			    {
			      hash ^= hi >> 24;
			      /* The ELF ABI says `hash &= ~hi', but
				 this is equivalent in this case and
				 on some machines one insn instead of
				 two.  */
			      hash ^= hi;
			    }
			}
		    }
		}
	    }
	}
    }
  return hash;
}


/* Inner part of the lookup functions.  */
static inline ElfW(Addr)
do_lookup (const char *undef_name, unsigned long int hash,
	   const ElfW(Sym) **ref, struct sym_val *result,
	   struct link_map *list[], size_t i, size_t n,
	   const char *reference_name, const hash_name_pair *version,
	   struct link_map *skip, int flags)
{
  struct link_map *map;

  for (; i < n; ++i)
    {
      const ElfW(Sym) *symtab;
      const char *strtab;
      const ElfW(Half) *verstab;
      ElfW(Symndx) symidx;

      map = list[i];

      /* Here come the extra test needed for `_dl_lookup_symbol_skip'.  */
      if (skip != NULL && map == skip)
	continue;

      /* Don't search the executable when resolving a copy reloc.  */
      if (flags & DL_LOOKUP_NOEXEC && map->l_type == lt_executable)
	continue;

      symtab = ((void *) map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
      strtab = ((void *) map->l_addr + map->l_info[DT_STRTAB]->d_un.d_ptr);
      if (version != NULL
	  && map->l_nversions > 0 && map->l_info[VERSTAG (DT_VERSYM)] != NULL)
	verstab = ((void *) map->l_addr
		   + map->l_info[VERSTAG (DT_VERSYM)]->d_un.d_ptr);
      else
	verstab = NULL;

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

	  if (verstab != NULL)
	    {
	      /* We can match the version information.  */
	      ElfW(Half) ndx = verstab[symidx] & 0x7fff;
	      if (map->l_versions[ndx].hash != version->hash
		  || strcmp (map->l_versions[ndx].name, version->name))
	      continue;
	    }

	  switch (ELFW(ST_BIND) (sym->st_info))
	    {
	    case STB_GLOBAL:
	      /* Global definition.  Just what we need.  */
	      result->s = sym;
	      result->a = map->l_addr;
	      return 1;
	    case STB_WEAK:
	      /* Weak definition.  Use this value if we don't find
		 another.  */
	      if (! result->s)
		{
		  result->s = sym;
		  result->a = map->l_addr;
		}
	      break;
	    default:
	      /* Local symbols are ignored.  */
	      break;
	    }
	}
    }

  /* We have not found anything until now.  */
  return 0;
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
  struct sym_val current_value = { 0, NULL };
  struct link_map **scope;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    if (do_lookup (undef_name, hash, ref, &current_value,
		   (*scope)->l_searchlist, 0, (*scope)->l_nsearchlist,
		   reference_name, NULL, NULL, flags))
      break;

  if (current_value.s == NULL &&
      (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK))
    {
      /* We could find no value for a strong reference.  */
      static const char msg[] = "undefined symbol: ";
      const size_t len = strlen (undef_name);
      char buf[sizeof msg + len];
      memcpy (buf, msg, sizeof msg - 1);
      memcpy (&buf[sizeof msg - 1], undef_name, len + 1);
      _dl_signal_error (0, reference_name, buf);
    }

  *ref = current_value.s;
  return current_value.a;
}


/* This function is nearly the same as `_dl_lookup_symbol' but it
   skips in the first list all objects until SKIP_MAP is found.  I.e.,
   it only considers objects which were loaded after the described
   object.  If there are more search lists the object described by
   SKIP_MAP is only skipped.  */
ElfW(Addr)
_dl_lookup_symbol_skip (const char *undef_name, const ElfW(Sym) **ref,
			struct link_map *symbol_scope[],
			const char *reference_name,
			struct link_map *skip_map,
			int flags)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { 0, NULL };
  struct link_map **scope;
  size_t i;

  /* Search the relevant loaded objects for a definition.  */
  scope = symbol_scope;
  for (i = 0; (*scope)->l_dupsearchlist[i] != skip_map; ++i)
    assert (i < (*scope)->l_ndupsearchlist);

  if (! do_lookup (undef_name, hash, ref, &current_value,
		   (*scope)->l_dupsearchlist, i, (*scope)->l_ndupsearchlist,
		   reference_name, NULL, skip_map, flags))
    while (*++scope)
      if (do_lookup (undef_name, hash, ref, &current_value,
		     (*scope)->l_dupsearchlist, 0, (*scope)->l_ndupsearchlist,
		     reference_name, NULL, skip_map, flags))
	break;

  *ref = current_value.s;
  return current_value.a;
}


/* This function works like _dl_lookup_symbol but it takes an
   additional arguement with the version number of the requested
   symbol.

   XXX We'll see whether we need this separate function.  */
ElfW(Addr)
_dl_lookup_versioned_symbol (const char *undef_name, const ElfW(Sym) **ref,
			     struct link_map *symbol_scope[],
			     const char *reference_name,
			     const hash_name_pair *version, int flags)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { 0, NULL };
  struct link_map **scope;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    if (do_lookup (undef_name, hash, ref, &current_value,
		   (*scope)->l_searchlist, 0, (*scope)->l_nsearchlist,
		   reference_name, version, NULL, flags))
      break;

  if (current_value.s == NULL &&
      (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK))
    {
      /* We could find no value for a strong reference.  */
      static const char msg1[] = "undefined symbol: ";
      const size_t len = strlen (undef_name);
      static const char msg2[] = ", version ";
      const size_t verslen = strlen (version->name ?: "") + 1;
      char buf[sizeof msg1 - 1 + len + sizeof msg2 - 1 + verslen];

      memcpy (buf, msg1, sizeof msg1 - 1);
      memcpy (&buf[sizeof msg1 - 1], undef_name, len + 1);
      memcpy (&buf[sizeof msg1 - 1 + len], msg2, sizeof msg2 - 1);
      memcpy (&buf[sizeof msg1 - 1 + len + sizeof msg2 - 1], version->name,
	      verslen);
      _dl_signal_error (0, reference_name, buf);
    }

  *ref = current_value.s;
  return current_value.a;
}


/* Similar to _dl_lookup_symbol_skip but takes an additional argument
   with the version we are looking for.  */
ElfW(Addr)
_dl_lookup_versioned_symbol_skip (const char *undef_name,
				  const ElfW(Sym) **ref,
				  struct link_map *symbol_scope[],
				  const char *reference_name,
				  const char *version_name,
				  struct link_map *skip_map,
				  int flags)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { 0, NULL };
  struct link_map **scope;
  hash_name_pair version;
  size_t i;

  /* First convert the VERSION_NAME into a `hash_name_pair' value.  */
  version.hash = _dl_elf_hash (version_name);
  version.name = version_name;

  /* Search the relevant loaded objects for a definition.  */
  scope = symbol_scope;
  for (i = 0; (*scope)->l_dupsearchlist[i] != skip_map; ++i)
    assert (i < (*scope)->l_ndupsearchlist);

  if (! do_lookup (undef_name, hash, ref, &current_value,
		   (*scope)->l_dupsearchlist, i, (*scope)->l_ndupsearchlist,
		   reference_name, &version, skip_map, flags))
    while (*++scope)
      if (do_lookup (undef_name, hash, ref, &current_value,
		     (*scope)->l_dupsearchlist, 0, (*scope)->l_ndupsearchlist,
		     reference_name, &version, skip_map, flags))
	break;

  *ref = current_value.s;
  return current_value.a;
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
