/* Locate the shared object symbol nearest a given address.
   Copyright (C) 1996, 1997, 1998, 1999, 2003, 2004
   Free Software Foundation, Inc.
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

#include <dlfcn.h>

#if !defined SHARED && defined IS_IN_libdl

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
  if (__builtin_expect (_dlfcn_hook != NULL, 0))
    return _dlfcn_hook->dladdr (address, info);
# endif
  return _dl_addr (address, info, NULL, NULL);
}
# ifdef SHARED
strong_alias (__dladdr, dladdr)
# endif
#endif
