/* Locate the shared object symbol nearest a given address.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
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

#include <dlfcn.h>

#if !defined SHARED && IS_IN (libdl)

int
dladdr (const void *address, Dl_info *info)
{
  return __dladdr (address, info);
}

#else

int
__dladdr (const void *address, Dl_info *info)
{
# ifdef SHARED
  if (__glibc_unlikely (_dlfcn_hook != NULL))
    return _dlfcn_hook->dladdr (address, info);
# endif
  return _dl_addr (address, info, NULL, NULL);
}
# ifdef SHARED
strong_alias (__dladdr, dladdr)
# endif
#endif
