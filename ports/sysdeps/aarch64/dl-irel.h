/* Machine-dependent ELF indirect relocation inline functions.
   AArch64 version.
   Copyright (C) 2012 Free Software Foundation, Inc.
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

#ifndef _DL_IREL_H
#define _DL_IREL_H

#include <stdio.h>
#include <unistd.h>

/* AArch64 does not yet implement IFUNC support.  However since
   2011-06-20 provision of a elf_ifunc_invoke has been mandatory.  */

static inline ElfW(Addr)
__attribute ((always_inline))
elf_ifunc_invoke (ElfW(Addr) addr)
{
  return ((ElfW(Addr) (*) (void)) (addr)) ();
}

#endif
