/* ELF symbol resolve functions for VDSO objects.
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

#include "config.h"
#include <ldsodefs.h>


void *
internal_function
_dl_vdso_vsym (const char *name, const struct r_found_version *vers)
{
  struct link_map *map = GLRO (dl_sysinfo_map);
  void *value = NULL;


  if (map != NULL)
    {
      /* Use a WEAK REF so we don't error out if the symbol is not found.  */
      ElfW (Sym) wsym;
      memset (&wsym, 0, sizeof (ElfW (Sym)));
      wsym.st_info = (unsigned char) ELFW (ST_INFO (STB_WEAK, STT_NOTYPE));

      /* Search the scope of the vdso map.  */
      const ElfW (Sym) *ref = &wsym;
      lookup_t result = GLRO (dl_lookup_symbol_x) (name, map, &ref,
						   map->l_local_scope,
						   vers, 0, 0, NULL);

      if (ref != NULL)
	value = DL_SYMBOL_ADDRESS (result, ref);
    }

  return value;
}
