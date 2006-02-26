/* Look up a symbol in the loaded objects.
   Copyright (C) 1995-2004, 2005, 2006 Free Software Foundation, Inc.
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

/* Inner part of the lookup functions.  We return a value > 0 if we
   found the symbol, the value 0 if nothing is found and < 0 if
   something bad happened.  */
static int
__attribute_noinline__
do_lookup_x (const char *undef_name, unsigned long int hash,
	     const ElfW(Sym) *ref, struct sym_val *result,
	     struct r_scope_elem *scope, size_t i,
	     const struct r_found_version *const version, int flags,
	     struct link_map *skip, int type_class)
{
  struct link_map **list = scope->r_list;
  size_t n = scope->r_nlist;
  struct link_map *map;

  do
    {
      const ElfW(Sym) *symtab;
      const char *strtab;
      const ElfW(Half) *verstab;
      Elf_Symndx symidx;
      const ElfW(Sym) *sym;
      int num_versions = 0;
      const ElfW(Sym) *versioned_sym = NULL;

      map = list[i]->l_real;

      /* Here come the extra test needed for `_dl_lookup_symbol_skip'.  */
      if (map == skip)
	continue;

      /* Don't search the executable when resolving a copy reloc.  */
      if ((type_class & ELF_RTYPE_CLASS_COPY) && map->l_type == lt_executable)
	continue;

      /* Do not look into objects which are going to be removed.  */
      if (map->l_removed)
	continue;

      /* Print some debugging info if wanted.  */
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS, 0))
	_dl_debug_printf ("symbol=%s;  lookup in file=%s [%lu]\n",
			  undef_name,
			  map->l_name[0] ? map->l_name : rtld_progname,
			  map->l_ns);

      symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
      strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
      verstab = map->l_versyms;

      /* Search the appropriate hash bucket in this object's symbol table
	 for a definition for the same symbol name.  */
      for (symidx = map->l_buckets[hash % map->l_nbuckets];
	   symidx != STN_UNDEF;
	   symidx = map->l_chain[symidx])
	{
	  sym = &symtab[symidx];

	  assert (ELF_RTYPE_CLASS_PLT == 1);
	  if ((sym->st_value == 0 /* No value.  */
#ifdef USE_TLS
	       && ELFW(ST_TYPE) (sym->st_info) != STT_TLS
#endif
	       )
	      || (type_class & (sym->st_shndx == SHN_UNDEF)))
	    continue;

	  if (ELFW(ST_TYPE) (sym->st_info) > STT_FUNC
#ifdef USE_TLS
	      && ELFW(ST_TYPE) (sym->st_info) != STT_TLS
#endif
	      )
	    /* Ignore all but STT_NOTYPE, STT_OBJECT and STT_FUNC
	       entries (and STT_TLS if TLS is supported) since these
	       are no code/data definitions.  */
	    continue;

	  if (sym != ref && strcmp (strtab + sym->st_name, undef_name))
	    /* Not the symbol we are looking for.  */
	    continue;

	  if (version != NULL)
	    {
	      if (__builtin_expect (verstab == NULL, 0))
		{
		  /* We need a versioned symbol but haven't found any.  If
		     this is the object which is referenced in the verneed
		     entry it is a bug in the library since a symbol must
		     not simply disappear.

		     It would also be a bug in the object since it means that
		     the list of required versions is incomplete and so the
		     tests in dl-version.c haven't found a problem.*/
		  assert (version->filename == NULL
			  || ! _dl_name_match_p (version->filename, map));

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
	  else
	    {
	      /* No specific version is selected.  There are two ways we
		 can got here:

		 - a binary which does not include versioning information
		   is loaded

		 - dlsym() instead of dlvsym() is used to get a symbol which
		   might exist in more than one form

		 If the library does not provide symbol version
		 information there is no problem at at: we simply use the
		 symbol if it is defined.

		 These two lookups need to be handled differently if the
		 library defines versions.  In the case of the old
		 unversioned application the oldest (default) version
		 should be used.  In case of a dlsym() call the latest and
		 public interface should be returned.  */
	      if (verstab != NULL)
		{
		  if ((verstab[symidx] & 0x7fff)
		      >= ((flags & DL_LOOKUP_RETURN_NEWEST) ? 2 : 3))
		    {
		      /* Don't accept hidden symbols.  */
		      if ((verstab[symidx] & 0x8000) == 0
			  && num_versions++ == 0)
			/* No version so far.  */
			versioned_sym = sym;

		      continue;
		    }
		}
	    }

	  /* There cannot be another entry for this symbol so stop here.  */
	  goto found_it;
	}

      /* If we have seen exactly one versioned symbol while we are
	 looking for an unversioned symbol and the version is not the
	 default version we still accept this symbol since there are
	 no possible ambiguities.  */
      sym = num_versions == 1 ? versioned_sym : NULL;

      if (sym != NULL)
	{
	found_it:
	  switch (ELFW(ST_BIND) (sym->st_info))
	    {
	    case STB_WEAK:
	      /* Weak definition.  Use this value if we don't find another.  */
	      if (__builtin_expect (GLRO(dl_dynamic_weak), 0))
		{
		  if (! result->s)
		    {
		      result->s = sym;
		      result->m = map;
		    }
		  break;
		}
	      /* FALLTHROUGH */
	    case STB_GLOBAL:
	      /* Global definition.  Just what we need.  */
	      result->s = sym;
	      result->m = map;
	      return 1;
	    default:
	      /* Local symbols are ignored.  */
	      break;
	    }
	}

      /* If this current map is the one mentioned in the verneed entry
	 and we have not found a weak entry, it is a bug.  */
      if (symidx == STN_UNDEF && version != NULL && version->filename != NULL
	  && __builtin_expect (_dl_name_match_p (version->filename, map), 0))
	return -1;
    }
  while (++i < n);

  /* We have not found anything until now.  */
  return 0;
}

#undef FCT
#undef ARG
#undef VERSIONED
