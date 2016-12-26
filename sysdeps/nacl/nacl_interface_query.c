/* NaCl function exposing IRT interface query.
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
#include <ldsodefs.h>

#ifdef SHARED

/* We can define this trivially using IFUNC rather than a wrapper
   because we absolutely require that we get the IRT interface query
   function pointer via AT_SYSINFO.  */

extern TYPE_nacl_irt_query nacl_interface_query_ifunc (void)
  asm ("nacl_interface_query");

TYPE_nacl_irt_query
inhibit_stack_protector
nacl_interface_query_ifunc (void)
{
  return &__nacl_irt_query;
}
asm (".type nacl_interface_query, %gnu_indirect_function");

#else

/* In the static library, using IFUNC is just extra overhead.  */

size_t
nacl_interface_query (const char *interface_ident,
                      void *table, size_t tablesize)
{
  return __nacl_irt_query (interface_ident, table, tablesize);
}

#endif
