/* Load a shared object at run time.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This file is for compatibility with glibc 2.0.  Compile it only if
   versioning is used.  */
#if defined PIC && DO_VERSIONING

#include <dlfcn.h>
#include <stddef.h>
#include <elf/ldsodefs.h>

struct dlopen_args
{
  /* The arguments for dlopen_doit.  */
  const char *file;
  int mode;
  /* The return value of dlopen_doit.  */
  struct link_map *new;
};


static void
dlopen_doit (void *a)
{
  struct dlopen_args *args = (struct dlopen_args *) a;

  args->new = _dl_open (args->file ?: "", args->mode);
}


void *
__dlopen_nocheck (const char *file, int mode)
{
  struct dlopen_args args;
  args.file = file;

  if ((mode & RTLD_BINDING_MASK) == 0)
    /* By default assume RTLD_LAZY.  */
    mode |= RTLD_LAZY;
  args.mode = mode;

  return _dlerror_run (dlopen_doit, &args) ? NULL : args.new;
}
symbol_version (__dlopen_nocheck, dlopen, GLIBC_2.0);
#endif
