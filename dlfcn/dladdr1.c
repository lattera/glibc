/* Locate the shared object symbol nearest a given address.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
dladdr1 (const void *address, Dl_info *info, void **extra, int flags)
{
  return __dladdr1 (address, info, extra, flags);
}

#else

int
__dladdr1 (const void *address, Dl_info *info, void **extra, int flags)
{
# ifdef SHARED
  if (__builtin_expect (_dlfcn_hook != NULL, 0))
    return _dlfcn_hook->dladdr1 (address, info, extra, flags);
# endif

  switch (flags)
    {
    default:			/* Make this an error?  */
    case 0:
      return _dl_addr (address, info, NULL, NULL);
    case RTLD_DL_SYMENT:
      return _dl_addr (address, info, NULL, (const ElfW(Sym) **) extra);
    case RTLD_DL_LINKMAP:
      return _dl_addr (address, info, (struct link_map **) extra, NULL);
    }
}
# ifdef SHARED
strong_alias (__dladdr1, dladdr1)
# endif
#endif
