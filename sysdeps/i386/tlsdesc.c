/* Manage TLS descriptors.  i386 version.
   Copyright (C) 2005-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <link.h>
#include <ldsodefs.h>
#include <elf/dynamic-link.h>
#include <tls.h>
#include <dl-tlsdesc.h>
#include <dl-unmap-segments.h>
#include <tlsdeschtab.h>

/* The following 4 functions take an entry_check_offset argument.
   It's computed by the caller as an offset between its entry point
   and the call site, such that by adding the built-in return address
   that is implicitly passed to the function with this offset, we can
   easily obtain the caller's entry point to compare with the entry
   point given in the TLS descriptor.  If it's changed, we want to
   return immediately.  */

/* This function is used to lazily resolve TLS_DESC REL relocations
   that reference the *ABS* segment in their own link maps.  The
   argument is the addend originally stored there.  */

void
__attribute__ ((regparm (3))) attribute_hidden
_dl_tlsdesc_resolve_abs_plus_addend_fixup (struct tlsdesc volatile *td,
					   struct link_map *l,
					   ptrdiff_t entry_check_offset)
{
  ptrdiff_t addend = (ptrdiff_t) td->arg;

  if (_dl_tlsdesc_resolve_early_return_p (td, __builtin_return_address (0)
					  - entry_check_offset))
    return;

#ifndef SHARED
  CHECK_STATIC_TLS (l, l);
#else
  if (!TRY_STATIC_TLS (l, l))
    {
      td->arg = _dl_make_tlsdesc_dynamic (l, addend);
      td->entry = _dl_tlsdesc_dynamic;
    }
  else
#endif
    {
      td->arg = (void*) (addend - l->l_tls_offset);
      td->entry = _dl_tlsdesc_return;
    }

  _dl_tlsdesc_wake_up_held_fixups ();
}

/* This function is used to lazily resolve TLS_DESC REL relocations
   that originally had zero addends.  The argument location, that
   originally held the addend, is used to hold a pointer to the
   relocation, but it has to be restored before we call the function
   that applies relocations.  */

void
__attribute__ ((regparm (3))) attribute_hidden
_dl_tlsdesc_resolve_rel_fixup (struct tlsdesc volatile *td,
			       struct link_map *l,
			       ptrdiff_t entry_check_offset)
{
  const ElfW(Rel) *reloc = td->arg;

  if (_dl_tlsdesc_resolve_early_return_p (td, __builtin_return_address (0)
					  - entry_check_offset))
    return;

  /* The code below was borrowed from _dl_fixup(),
     except for checking for STB_LOCAL.  */
  const ElfW(Sym) *const symtab
    = (const void *) D_PTR (l, l_info[DT_SYMTAB]);
  const char *strtab = (const void *) D_PTR (l, l_info[DT_STRTAB]);
  const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];
  lookup_t result;

   /* Look up the target symbol.  If the normal lookup rules are not
      used don't look in the global scope.  */
  if (ELFW(ST_BIND) (sym->st_info) != STB_LOCAL
      && __builtin_expect (ELFW(ST_VISIBILITY) (sym->st_other), 0) == 0)
    {
      const struct r_found_version *version = NULL;

      if (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	{
	  const ElfW(Half) *vernum =
	    (const void *) D_PTR (l, l_info[VERSYMIDX (DT_VERSYM)]);
	  ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)] & 0x7fff;
	  version = &l->l_versions[ndx];
	  if (version->hash == 0)
	    version = NULL;
	}

      result = _dl_lookup_symbol_x (strtab + sym->st_name, l, &sym,
				    l->l_scope, version, ELF_RTYPE_CLASS_PLT,
				    DL_LOOKUP_ADD_DEPENDENCY, NULL);
    }
  else
    {
      /* We already found the symbol.  The module (and therefore its load
	 address) is also known.  */
      result = l;
    }

  if (!sym)
    {
      td->arg = 0;
      td->entry = _dl_tlsdesc_undefweak;
    }
  else
    {
#  ifndef SHARED
      CHECK_STATIC_TLS (l, result);
#  else
      if (!TRY_STATIC_TLS (l, result))
	{
	  td->arg = _dl_make_tlsdesc_dynamic (result, sym->st_value);
	  td->entry = _dl_tlsdesc_dynamic;
	}
      else
#  endif
	{
	  td->arg = (void*)(sym->st_value - result->l_tls_offset);
	  td->entry = _dl_tlsdesc_return;
	}
    }

  _dl_tlsdesc_wake_up_held_fixups ();
}

/* This function is used to lazily resolve TLS_DESC RELA relocations.
   The argument location is used to hold a pointer to the relocation.  */

void
__attribute__ ((regparm (3))) attribute_hidden
_dl_tlsdesc_resolve_rela_fixup (struct tlsdesc volatile *td,
				struct link_map *l,
				ptrdiff_t entry_check_offset)
{
  const ElfW(Rela) *reloc = td->arg;

  if (_dl_tlsdesc_resolve_early_return_p (td, __builtin_return_address (0)
					  - entry_check_offset))
    return;

  /* The code below was borrowed from _dl_fixup(),
     except for checking for STB_LOCAL.  */
  const ElfW(Sym) *const symtab
    = (const void *) D_PTR (l, l_info[DT_SYMTAB]);
  const char *strtab = (const void *) D_PTR (l, l_info[DT_STRTAB]);
  const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];
  lookup_t result;

   /* Look up the target symbol.  If the normal lookup rules are not
      used don't look in the global scope.  */
  if (ELFW(ST_BIND) (sym->st_info) != STB_LOCAL
      && __builtin_expect (ELFW(ST_VISIBILITY) (sym->st_other), 0) == 0)
    {
      const struct r_found_version *version = NULL;

      if (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	{
	  const ElfW(Half) *vernum =
	    (const void *) D_PTR (l, l_info[VERSYMIDX (DT_VERSYM)]);
	  ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)] & 0x7fff;
	  version = &l->l_versions[ndx];
	  if (version->hash == 0)
	    version = NULL;
	}

      result = _dl_lookup_symbol_x (strtab + sym->st_name, l, &sym,
				    l->l_scope, version, ELF_RTYPE_CLASS_PLT,
				    DL_LOOKUP_ADD_DEPENDENCY, NULL);
    }
  else
    {
      /* We already found the symbol.  The module (and therefore its load
	 address) is also known.  */
      result = l;
    }

  if (!sym)
    {
      td->arg = (void*) reloc->r_addend;
      td->entry = _dl_tlsdesc_undefweak;
    }
  else
    {
#  ifndef SHARED
      CHECK_STATIC_TLS (l, result);
#  else
      if (!TRY_STATIC_TLS (l, result))
	{
	  td->arg = _dl_make_tlsdesc_dynamic (result, sym->st_value
					      + reloc->r_addend);
	  td->entry = _dl_tlsdesc_dynamic;
	}
      else
#  endif
	{
	  td->arg = (void*) (sym->st_value - result->l_tls_offset
			     + reloc->r_addend);
	  td->entry = _dl_tlsdesc_return;
	}
    }

  _dl_tlsdesc_wake_up_held_fixups ();
}

/* This function is used to avoid busy waiting for other threads to
   complete the lazy relocation.  Once another thread wins the race to
   relocate a TLS descriptor, it sets the descriptor up such that this
   function is called to wait until the resolver releases the
   lock.  */

void
__attribute__ ((regparm (3))) attribute_hidden
_dl_tlsdesc_resolve_hold_fixup (struct tlsdesc volatile *td,
				struct link_map *l __attribute__((__unused__)),
				ptrdiff_t entry_check_offset)
{
  /* Maybe we're lucky and can return early.  */
  if (__builtin_return_address (0) - entry_check_offset != td->entry)
    return;

  /* Locking here will stop execution until the running resolver runs
     _dl_tlsdesc_wake_up_held_fixups(), releasing the lock.

     FIXME: We'd be better off waiting on a condition variable, such
     that we didn't have to hold the lock throughout the relocation
     processing.  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));
  __rtld_lock_unlock_recursive (GL(dl_load_lock));
}


/* Unmap the dynamic object, but also release its TLS descriptor table
   if there is one.  */

void
internal_function
_dl_unmap (struct link_map *map)
{
  _dl_unmap_segments (map);

#ifdef SHARED
  if (map->l_mach.tlsdesc_table)
    htab_delete (map->l_mach.tlsdesc_table);
#endif
}
