/* Look up a symbol in the loaded objects.
   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

/* Statistics function.  */
unsigned long int _dl_num_relocations;


/* We have two different situations when looking up a simple: with or
   without versioning.  gcc is not able to optimize a single function
   definition serving for both purposes so we define two functions.  */
#define VERSIONED	0
#include "do-lookup.h"

#define VERSIONED	1
#include "do-lookup.h"


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

  ++_dl_num_relocations;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    if (do_lookup (undef_name, hash, *ref, &current_value,
		   *scope, 0, reference_name, NULL, reloc_type))
      break;

  if (current_value.s == NULL)
    {
      if (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	/* We could find no value for a strong reference.  */
	_dl_signal_cerror (0, (reference_name  && reference_name[0]
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

  ++_dl_num_relocations;

  /* Search the relevant loaded objects for a definition.  */
  scope = symbol_scope;
  for (i = 0; (*scope)->r_duplist[i] != skip_map; ++i)
    assert (i < (*scope)->r_nduplist);

  if (i >= (*scope)->r_nlist
      || ! do_lookup (undef_name, hash, *ref, &current_value,
		      *scope, i, reference_name, skip_map, 0))
    while (*++scope)
      if (do_lookup (undef_name, hash, *ref, &current_value,
		     *scope, 0, reference_name, skip_map, 0))
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

  ++_dl_num_relocations;

  /* Search the relevant loaded objects for a definition.  */
  for (scope = symbol_scope; *scope; ++scope)
    {
      int res = do_lookup_versioned (undef_name, hash, *ref, &current_value,
				     *scope, 0, reference_name, version, NULL,
				     reloc_type);
      if (res > 0)
	break;

      if (res < 0)
	{
	  /* Oh, oh.  The file named in the relocation entry does not
	     contain the needed symbol.  */
	  _dl_signal_cerror (0, (reference_name && reference_name[0]
				 ? reference_name
				 : (_dl_argv[0] ?: "<main program>")),
			     make_string ("symbol ", undef_name, ", version ",
					  version->name,
					  " not defined in file ",
					  version->filename,
					  " with link time reference",
					  res == -2
					  ? " (no version symbols)" : ""));
	  *ref = NULL;
	  return 0;
	}
    }

  if (current_value.s == NULL)
    {
      if (*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	/* We could find no value for a strong reference.  */
	_dl_signal_cerror (0, (reference_name && reference_name[0]
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

  ++_dl_num_relocations;

  /* Search the relevant loaded objects for a definition.  */
  scope = symbol_scope;
  for (i = 0; (*scope)->r_duplist[i] != skip_map; ++i)
    assert (i < (*scope)->r_nduplist);

  if (i >= (*scope)->r_nlist
      || ! do_lookup_versioned (undef_name, hash, *ref, &current_value, *scope,
				i, reference_name, version, skip_map, 0))
    while (*++scope)
      if (do_lookup_versioned (undef_name, hash, *ref, &current_value, *scope,
			       0, reference_name, version, skip_map, 0))
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
	  _dl_signal_cerror (0, (reference_name && reference_name[0]
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
