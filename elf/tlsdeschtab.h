/* Hash table for TLS descriptors.
   Copyright (C) 2005, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva  <aoliva@redhat.com>

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

#ifndef TLSDESCHTAB_H
# define TLSDESCHTAB_H 1

# ifdef SHARED

#  include <inline-hashtab.h>

inline static int
hash_tlsdesc (void *p)
{
  struct tlsdesc_dynamic_arg *td = p;

  /* We know all entries are for the same module, so ti_offset is the
     only distinguishing entry.  */
  return td->tlsinfo.ti_offset;
}

inline static int
eq_tlsdesc (void *p, void *q)
{
  struct tlsdesc_dynamic_arg *tdp = p, *tdq = q;

  return tdp->tlsinfo.ti_offset == tdq->tlsinfo.ti_offset;
}

inline static int
map_generation (struct link_map *map)
{
  size_t idx = map->l_tls_modid;
  struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);

  /* Find the place in the dtv slotinfo list.  */
  do
    {
      /* Does it fit in the array of this list element?  */
      if (idx < listp->len)
	{
	  /* We should never get here for a module in static TLS, so
	     we can assume that, if the generation count is zero, we
	     still haven't determined the generation count for this
	     module.  */
	  if (listp->slotinfo[idx].gen)
	    return listp->slotinfo[idx].gen;
	  else
	    break;
	}
      idx -= listp->len;
      listp = listp->next;
    }
  while (listp != NULL);

  /* If we get to this point, the module still hasn't been assigned an
     entry in the dtv slotinfo data structures, and it will when we're
     done with relocations.  At that point, the module will get a
     generation number that is one past the current generation, so
     return exactly that.  */
  return GL(dl_tls_generation) + 1;
}

void *
internal_function
_dl_make_tlsdesc_dynamic (struct link_map *map, size_t ti_offset)
{
  struct hashtab *ht;
  void **entry;
  struct tlsdesc_dynamic_arg *td, test;

  /* FIXME: We could use a per-map lock here, but is it worth it?  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));

  ht = map->l_mach.tlsdesc_table;
  if (! ht)
    {
      ht = htab_create ();
      if (! ht)
	{
	  __rtld_lock_unlock_recursive (GL(dl_load_lock));
	  return 0;
	}
      map->l_mach.tlsdesc_table = ht;
    }

  test.tlsinfo.ti_module = map->l_tls_modid;
  test.tlsinfo.ti_offset = ti_offset;
  entry = htab_find_slot (ht, &test, 1, hash_tlsdesc, eq_tlsdesc);
  if (*entry)
    {
      td = *entry;
      __rtld_lock_unlock_recursive (GL(dl_load_lock));
      return td;
    }

  *entry = td = malloc (sizeof (struct tlsdesc_dynamic_arg));
  /* This may be higher than the map's generation, but it doesn't
     matter much.  Worst case, we'll have one extra DTV update per
     thread.  */
  td->gen_count = map_generation (map);
  td->tlsinfo = test.tlsinfo;

  __rtld_lock_unlock_recursive (GL(dl_load_lock));
  return td;
}

# endif /* SHARED */

/* The idea of the following two functions is to stop multiple threads
   from attempting to resolve the same TLS descriptor without busy
   waiting.  Ideally, we should be able to release the lock right
   after changing td->entry, and then using say a condition variable
   or a futex wake to wake up any waiting threads, but let's try to
   avoid introducing such dependencies.  */

inline static int
_dl_tlsdesc_resolve_early_return_p (struct tlsdesc volatile *td, void *caller)
{
  if (caller != td->entry)
    return 1;

  __rtld_lock_lock_recursive (GL(dl_load_lock));
  if (caller != td->entry)
    {
      __rtld_lock_unlock_recursive (GL(dl_load_lock));
      return 1;
    }

  td->entry = _dl_tlsdesc_resolve_hold;

  return 0;
}

inline static void
_dl_tlsdesc_wake_up_held_fixups (void)
{
  __rtld_lock_unlock_recursive (GL(dl_load_lock));
}

#endif
