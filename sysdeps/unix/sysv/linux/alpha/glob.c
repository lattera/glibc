/* Copyright (C) 1998 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

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

#include <sysdeps/generic/glob.c>

#undef glob
#undef globfree

default_symbol_version(__new_glob, glob, GLIBC_2.1);
default_symbol_version(__new_globfree, globfree, GLIBC_2.1);
