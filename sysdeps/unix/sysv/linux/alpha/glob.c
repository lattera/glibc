/* Copyright (C) 1998-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define glob64 __no_glob64_decl
#define globfree64 __no_globfree64_decl

#include <sys/types.h>
#include <glob.h>
#include <shlib-compat.h>

/* For Linux/Alpha we have to make the glob symbols versioned.  */
#define glob(pattern, flags, errfunc, pglob) \
  __new_glob (pattern, flags, errfunc, pglob)
#define globfree(pglob) \
  __new_globfree (pglob)

/* We need prototypes for these new names.  */
extern int __new_glob (const char *__pattern, int __flags,
		       int (*__errfunc) (const char *, int),
		       glob_t *__pglob);
extern void __new_globfree (glob_t *__pglob);

#include <posix/glob.c>

#undef glob
#undef globfree
#undef glob64
#undef globfree64

versioned_symbol (libc, __new_glob, glob, GLIBC_2_1);
versioned_symbol (libc, __new_globfree, globfree, GLIBC_2_1);
libc_hidden_ver (__new_glob, glob)
libc_hidden_ver (__new_globfree, globfree)

weak_alias (__new_glob, glob64)
weak_alias (__new_globfree, globfree64)
libc_hidden_ver (__new_globfree, globfree64)
