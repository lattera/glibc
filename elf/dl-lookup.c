/* Look up a symbol in the loaded objects.
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

#include <alloca.h>
#include <string.h>
#include <unistd.h>
#include <elf/ldsodefs.h>
#include "dl-hash.h"
#include <dl-machine.h>

#include <assert.h>

#define VERSTAG(tag)	(DT_NUM + DT_PROCNUM + DT_VERSIONTAGIDX (tag))

/* We need this string more than once.  */
static const char undefined_msg[] = "undefined symbol: ";


struct sym_val
  {
    const ElfW(Sym) *s;
    struct link_map *m;
  };


#define make_string(string, rest...) \
  ({									      \
    const char *all[] = { string, ## rest };				      \
    size_t len, cnt;							      \
    char *result, *cp;							      \
									      \
    len = 1;								      \
    for (cnt = 0; cnt < sizeof (all) / sizeof (all[0]); ++cnt)		      \
      len += strlen (all[cnt]);						      \
									      \
    cp = result = alloca (len);						      \
    for (cnt = 0; cnt < sizeof (all) / sizeof (all[0]); ++cnt)		      \
      cp = __stpcpy (cp, all[cnt]);					      \
									      \
    result;								      \
  })


/* Inner part of the lookup functions.  We return a value > 0 if we
   found the symbol, the value 0 if nothing is found and < 0 if
   something bad happened.  */
static inline int
do_lookup (const char *undef_name, unsigned long int hash,
	   const ElfW(Sym) *ref, struct sym_val *result,
	   struct r_scope_elem *scope, size_t i, const char *reference_name,
	   const struct r_found_version *version, struct link_map *skip,
	   int reloc_type)
{
  struct link_map **list = scope->r_list;
  size_t n = scope->r_nlist;
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

      /* Skip objects that could not be opened, which can occur in trace
	 mode.  */
      if (map->l_opencount == 0)
	continue;

      /* Don't search the executable when resolving a copy reloc.  */
      if (elf_machine_lookup_noexec_p (reloc_type)
	  && map->l_type == lt_executable)
	continue;

      /* Skip objects without symbol tables.  */
      if (map->l_info[DT_SYMTAB] == NULL)
	continue;

      /* Print some debugging info if wanted.  */
      if (_dl_debug_symbols)
	_dl_debug_message (1, "symbol=", undef_name, ";  lookup in file=",
			   map->l_name[0] ? map->l_name : _dl_argv[0],
			   "\n", NULL);

      symtab = ((void *) map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
      strtab = ((void *) map->l_addr + map->l_info[DT_STRTAB]->d_un.d_ptr);
      verstab = map->l_versyms;

      /* Search the appropriate hash bucket in this object's symbol table
	 for a definition for the same symbol name.  */
      for (symidx = map->l_buckets[hash % map->l_nbuckets];
	   symidx != STN_UNDEF;
	   symidx = map->l_chain[symidx])
	{
	  const ElfW(Sym) *sym = &symtab[symidx];

	  if (sym->st_value == 0 || /* No value.  */
	      (elf_machine_lookup_noplt_p (reloc_type) /* Reject PLT entry.  */
	       && sym->st_shndx == SHN_UNDEF))
	    continue;

	  if (ELFW(ST_TYPE) (sym->st_info) > STT_FUNC)
	    /* Ignore all but STT_NOTYPE, STT_OBJECT and STT_FUNC entries
	       since these are no code/data definitions.  */
	    continue;

	  if (sym != ref && strcmp (strtab + sym->st_name, undef_name))
	    /* Not the symbol we are looking for.  */
	    continue;

	  if (version == NULL)
	    {
	      /* No specific version is selected.  When the object
		 file also does not define a version we have a match.
		 Otherwise we only accept the default version, i.e.,
		 the version which name is "".  */
	      if (verstab != NULL)
		{
		  ElfW(Half) ndx = verstab[symidx] & 0x7fff;
		  if (ndx > 2) /* map->l_versions[ndx].hash != 0) */
		    continue;
		}
	    }
	  else
	    {
	      if (verstab == NULL)
		{
		  /* We need a versioned system but haven't found any.
		     If this is the object which is referenced in the
		     verneed entry it is a bug in the library since a
		     symbol must not simply disappear.  */
		  if (version->filename != NULL
		      && _dl_name_match_p (version->filename, map))
		    return -2;
		  /* Otherwise we accept the symbol.  */
		}
	      else
		{
		  /* We can match the version information or use the
		     default one if it is not hidden.  */
		  ElfW(Half) ndx = verstab[symidx] & 0x7fff;
		  if ((map->l_versions[ndx].hash != version->hash
		       || strcmp (map->l_versions[ndx].name, version->name))
		      && (version->hidden || map->l_versions[ndx].hash
			  || (verstab[symidx] & 0x8000)))
		    /* It's not the version we want.  */
		    continue;
		}
	    }

	  switch (ELFW(ST_BIND) (sym->st_info))
	    {
	    case STB_GLOBAL:
	      /* Global definition.  Just what we need.  */
	      result->s = sym;
	      result->m = map;
	      return 1;
	    case STB_WEAK:
	      /* Weak definition.  Use this value if we don't find
		 another.  */
	      if (! result->s)
		{
		  result->s = sym;
		  result->m = map;
		}
	      break;
	    default:
	      /* Local symbols are ignored.  */
	      break;
	    }

	  /* There cannot be another entry for this symbol so stop here.  */
	  break;
	}

      /* If this current map is the one mentioned in the verneed entry
	 and we have not found a weak entry, it is a bug.  */
      if (symidx == STN_UNDEF && version != NULL && version->filename != NULL
	  && _dl_name_match_p (version->filename, map))
	return -1;
    }

  /* We have not found anything until now.  */
  return 0;
}

/* Search loaded objects' symbol tables for a definition of the symbol
   UNDEF_NAME.  */

ElfW(Addr)
internal_function
_dl_lookup_symbol (const char *undef_name, const ElfW(Sym) **ref,
		   struct r_scope_elem *symbol_scope[],
		   const char *reference_name,
		   int reloc_type)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { NULL, NULL };
  struct r_scope_elem **scope;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    if (do_lookup (undef_name, hash, *ref, &current_value,
		   *scope, 0, reference_name, NULL, NULL, reloc_type))
      break;

  if (current_value.s == NULL)
    {
      if (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	/* We could find no value for a strong reference.  */
	_dl_signal_error (0, (reference_name  && reference_name[0]
			      ? reference_name
			      : (_dl_argv[0] ?: "<main program>")),
			  make_string (undefined_msg, undef_name));
      *ref = NULL;
      return 0;
    }

  if (_dl_debug_bindings)
    _dl_debug_message (1, "binding file ",
		       (reference_name && reference_name[0]
			? reference_name
			: (_dl_argv[0] ?: "<main program>")),
		       " to ", current_value.m->l_name[0]
		       ? current_value.m->l_name : _dl_argv[0],
		       ": symbol `", undef_name, "'\n", NULL);

  *ref = current_value.s;
  return current_value.m->l_addr;
}


/* This function is nearly the same as `_dl_lookup_symbol' but it
   skips in the first list all objects until SKIP_MAP is found.  I.e.,
   it only considers objects which were loaded after the described
   object.  If there are more search lists the object described by
   SKIP_MAP is only skipped.  */
ElfW(Addr)
internal_function
_dl_lookup_symbol_skip (const char *undef_name, const ElfW(Sym) **ref,
			struct r_scope_elem *symbol_scope[],
			const char *reference_name,
			struct link_map *skip_map)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { NULL, NULL };
  struct r_scope_elem **scope;
  size_t i;

  /* Search the relevant loaded objects for a definition.  */
  scope = symbol_scope;
  for (i = 0; (*scope)->r_duplist[i] != skip_map; ++i)
    assert (i < (*scope)->r_nduplist);

  if (! do_lookup (undef_name, hash, *ref, &current_value,
		   *scope, i, reference_name, NULL, skip_map, 0))
    while (*++scope)
      if (do_lookup (undef_name, hash, *ref, &current_value,
		     *scope, 0, reference_name, NULL, skip_map, 0))
	break;

  if (current_value.s == NULL)
    {
      *ref = NULL;
      return 0;
    }

  if (_dl_debug_bindings)
    _dl_debug_message (1, "binding file ",
		       (reference_name && reference_name[0]
			? reference_name
			: (_dl_argv[0] ?: "<main program>")),
		       " to ", current_value.m->l_name[0]
		       ? current_value.m->l_name : _dl_argv[0],
		       ": symbol `", undef_name, "' (skip)\n", NULL);

  *ref = current_value.s;
  return current_value.m->l_addr;
}


/* This function works like _dl_lookup_symbol but it takes an
   additional arguement with the version number of the requested
   symbol.

   XXX We'll see whether we need this separate function.  */
ElfW(Addr)
internal_function
_dl_lookup_versioned_symbol (const char *undef_name, const ElfW(Sym) **ref,
			     struct r_scope_elem *symbol_scope[],
			     const char *reference_name,
			     const struct r_found_version *version,
			     int reloc_type)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { NULL, NULL };
  struct r_scope_elem **scope;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    {
      int res = do_lookup (undef_name, hash, *ref, &current_value,
			   *scope, 0, reference_name, version, NULL, reloc_type);
      if (res > 0)
	break;

      if (res < 0)
	/* Oh, oh.  The file named in the relocation entry does not
	   contain the needed symbol.  */
	_dl_signal_error (0, (reference_name && reference_name[0]
			      ? reference_name
			      : (_dl_argv[0] ?: "<main program>")),
			  make_string ("symbol ", undef_name, ", version ",
				       version->name,
				       " not defined in file ",
				       version->filename,
				       " with link time reference",
				       res == -2
				       ? " (no version symbols)" : ""));
    }

  if (current_value.s == NULL)
    {
      if (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	/* We could find no value for a strong reference.  */
	_dl_signal_error (0, (reference_name && reference_name[0]
			      ? reference_name
			      : (_dl_argv[0] ?: "<main program>")),
			  make_string (undefined_msg, undef_name,
				       ", version ", version->name ?: NULL));
      *ref = NULL;
      return 0;
    }

  if (_dl_debug_bindings)
    _dl_debug_message (1, "binding file ",
		       (reference_name && reference_name[0]
			? reference_name
			: (_dl_argv[0] ?: "<main program>")),
		       " to ", current_value.m->l_name[0]
		       ? current_value.m->l_name : _dl_argv[0],
		       ": symbol `", undef_name, "' [", version->name,
		       "]\n", NULL);

  *ref = current_value.s;
  return current_value.m->l_addr;
}


/* Similar to _dl_lookup_symbol_skip but takes an additional argument
   with the version we are looking for.  */
ElfW(Addr)
internal_function
_dl_lookup_versioned_symbol_skip (const char *undef_name,
				  const ElfW(Sym) **ref,
				  struct r_scope_elem *symbol_scope[],
				  const char *reference_name,
				  const struct r_found_version *version,
				  struct link_map *skip_map)
{
  const unsigned long int hash = _dl_elf_hash (undef_name);
  struct sym_val current_value = { NULL, NULL };
  struct r_scope_elem **scope;
  size_t i;

  /* Search the relevant loaded objects for a definition.  */
  scope = symbol_scope;
  for (i = 0; (*scope)->r_duplist[i] != skip_map; ++i)
    assert (i < (*scope)->r_nduplist);

  if (! do_lookup (undef_name, hash, *ref, &current_value,
		   *scope, i, reference_name, version, skip_map, 0))
    while (*++scope)
      if (do_lookup (undef_name, hash, *ref, &current_value,
		     *scope, 0, reference_name, version, skip_map, 0))
	break;

  if (current_value.s == NULL)
    {
      if (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	{
	  /* We could find no value for a strong reference.  */
	  const size_t len = strlen (undef_name);
	  char buf[sizeof undefined_msg + len];
	  __mempcpy (__mempcpy (buf, undefined_msg, sizeof undefined_msg - 1),
		     undef_name, len + 1);
	  _dl_signal_error (0, (reference_name && reference_name[0]
				? reference_name
				: (_dl_argv[0] ?: "<main program>")), buf);
	}
      *ref = NULL;
      return 0;
    }

  if (_dl_debug_bindings)
    _dl_debug_message (1, "binding file ",
		       (reference_name && reference_name[0]
			? reference_name
			: (_dl_argv[0] ?: "<main program>")),
		       " to ",
		       current_value.m->l_name[0]
		       ? current_value.m->l_name : _dl_argv[0],
		       ": symbol `", undef_name, "' [", version->name,
		       "] (skip)\n", NULL);

  *ref = current_value.s;
  return current_value.m->l_addr;
}


/* Cache the location of MAP's hash table.  */

void
internal_function
_dl_setup_hash (struct link_map *map)
{
  ElfW(Symndx) *hash;
  ElfW(Symndx) nchain;

  if (!map->l_info[DT_HASH])
    return;
  hash = (void *)(map->l_addr + map->l_info[DT_HASH]->d_un.d_ptr);

  map->l_nbuckets = *hash++;
  nchain = *hash++;
  map->l_buckets = hash;
  hash += map->l_nbuckets;
  map->l_chain = hash;
}
