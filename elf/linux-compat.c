/* Initializer for Linux-compatible dynamic linker `/lib/ld-linux.so.1'.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <link.h>
#include <stdlib.h>

/* This function will be the DT_INIT initializer for the ld-linux.so.1
   shared object.  This is called from rtld.c before shlib initializers.

   The old Linux ELF startup code expects the dynamic linker to magically
   call atexit to arrange for shared object finalizers to run.  (The
   ABI-compliant startup code does this itself.)  We build a compatible
   version of the dynamic linker to install as /lib/ld-linux.so.1, the
   name old Linux ELF binaries use.  */

void
_init (void)
{
  const Elf32_Sym *ref = NULL;
  Elf32_Addr loadbase = _dl_lookup_symbol ("atexit", &ref, _dl_loaded,
					   "<ld-linux.so.1 initialization>",
					   1);
  (*(__typeof (atexit) *) (loadbase + ref->st_value)) (&_dl_fini);
}
