/* Helper definitions for profiling of shared libraries.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <dlfcn.h>
#include <elf.h>
#include <elf/ldsodefs.h>

/* This is the map for the shared object we profile.  It is defined here
   only because we test for this value being NULL or not.  */
struct link_map *_dl_profile_map;


void
_dl_mcount_wrapper (ElfW(Addr) selfpc)
{
  _dl_mcount ((ElfW(Addr)) __builtin_return_address (0), selfpc);
}


void
_dl_mcount_wrapper_check (void *selfpc)
{
  if (_dl_profile_map != NULL)
    _dl_mcount ((ElfW(Addr)) __builtin_return_address (0),
		(ElfW(Addr)) selfpc);
}
