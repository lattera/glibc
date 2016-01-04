/* Interface for the user to replace NaCl IRT interface functions.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <nacl-interfaces.h>
#include <string.h>

size_t
nacl_interface_ext_supply (const char *interface_ident,
			   const void *table, size_t tablesize)
{
  const size_t ident_len = strlen (interface_ident) + 1;

  /* Most interfaces are in rtld, so try there first.  If other
     libraries ever get their own tables not used in libc, then we
     will need some dynamic registration mechanism here to iterate
     over all libraries' __nacl_supply_interface_libfoo calls.  */
  if (0
#ifdef SHARED
      || __nacl_supply_interface_rtld (interface_ident, ident_len,
				       table, tablesize)
#endif
      || __nacl_supply_interface_libc (interface_ident, ident_len,
				       table, tablesize))
    return tablesize;

  return 0;
}
