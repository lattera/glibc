/* Look up a symbol in a shared object loaded by `dlopen'.
   Copyright (C) 1999, 2000, 2001, 2002, 2004 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <setjmp.h>
#include <libintl.h>

#include <dlfcn.h>
#include <ldsodefs.h>
#include <dl-hash.h>
#ifdef USE_TLS
# include <dl-tls.h>
#endif


#if defined USE_TLS && defined SHARED
/* Systems which do not have tls_index also probably have to define
   DONT_USE_TLS_INDEX.  */

# ifndef __TLS_GET_ADDR
#  define __TLS_GET_ADDR __tls_get_addr
# endif

/* Return the symbol address given the map of the module it is in and
   the symbol record.  This is used in dl-sym.c.  */
static void *
internal_function
_dl_tls_symaddr (struct link_map *map, const ElfW(Sym) *ref)
{
# ifndef DONT_USE_TLS_INDEX
  tls_index tmp =
    {
      .ti_module = map->l_tls_modid,
      .ti_offset = ref->st_value
    };

  return __TLS_GET_ADDR (&tmp);
# else
  return __TLS_GET_ADDR (map->l_tls_modid, ref->st_value);
# endif
}
#endif


static void *
internal_function
do_sym (void *handle, const char *name, void *who,
	struct r_found_version *vers, int flags)
{
  const ElfW(Sym) *ref = NULL;
  lookup_t result;
  ElfW(Addr) caller = (ElfW(Addr)) who;

  /* If the address is not recognized the call comes from the main
     program (we hope).  */
  struct link_map *match = GL(dl_ns)[LM_ID_BASE]._ns_loaded;

  /* Find the highest-addressed object that CALLER is not below.  */
  for (Lmid_t ns = 0; ns < DL_NNS; ++ns)
    for (struct link_map *l = GL(dl_ns)[ns]._ns_loaded; l != NULL;
	 l = l->l_next)
      if (caller >= l->l_map_start && caller < l->l_map_end)
	{
	  /* There must be exactly one DSO for the range of the virtual
	     memory.  Otherwise something is really broken.  */
	  match = l;
	  break;
	}

  if (handle == RTLD_DEFAULT)
    /* Search the global scope.  */
    result = GLRO(dl_lookup_symbol_x) (name, match, &ref, match->l_scope,
				       vers, 0, flags|DL_LOOKUP_ADD_DEPENDENCY,
				       NULL);
  else if (handle == RTLD_NEXT)
    {
      if (__builtin_expect (match == GL(dl_ns)[LM_ID_BASE]._ns_loaded, 0))
	{
	  if (match == NULL
	      || caller < match->l_map_start
	      || caller >= match->l_map_end)
	    GLRO(dl_signal_error) (0, NULL, NULL, N_("\
RTLD_NEXT used in code not dynamically loaded"));
	}

      struct link_map *l = match;
      while (l->l_loader != NULL)
	l = l->l_loader;

      result = GLRO(dl_lookup_symbol_x) (name, l, &ref, l->l_local_scope,
					 vers, 0, 0, match);
    }
  else
    {
      /* Search the scope of the given object.  */
      struct link_map *map = handle;
      result = GLRO(dl_lookup_symbol_x) (name, map, &ref, map->l_local_scope,
					 vers, 0, flags, NULL);
    }

  if (ref != NULL)
    {
#if defined USE_TLS && defined SHARED
      if (ELFW(ST_TYPE) (ref->st_info) == STT_TLS)
	/* The found symbol is a thread-local storage variable.
	   Return the address for to the current thread.  */
	return _dl_tls_symaddr (result, ref);
#endif

      return DL_SYMBOL_ADDRESS (result, ref);
    }

  return NULL;
}


void *
internal_function
_dl_vsym (void *handle, const char *name, const char *version, void *who)
{
  struct r_found_version vers;

  /* Compute hash value to the version string.  */
  vers.name = version;
  vers.hidden = 1;
  vers.hash = _dl_elf_hash (version);
  /* We don't have a specific file where the symbol can be found.  */
  vers.filename = NULL;

  return do_sym (handle, name, who, &vers, 0);
}


void *
internal_function
_dl_sym (void *handle, const char *name, void *who)
{
  return do_sym (handle, name, who, NULL, DL_LOOKUP_RETURN_NEWEST);
}
