/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <sizes.h>
#include <errlist.h>

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING

# define SYS_ERRLIST __new_sys_errlist
# define SYS_NERR __new_sys_nerr

asm (".data; .globl __old_sys_errlist;  __old_sys_errlist:");
#endif

#include <sysdeps/gnu/errlist.c>

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
asm (".type __old_sys_errlist,@object;.size __old_sys_errlist,"
     OLD_ERRLIST_SIZE_STR "*" PTR_SIZE_STR);

extern const char *const *__old_sys_errlist;

const int __old_sys_nerr = OLD_ERRLIST_SIZE;

strong_alias (__old_sys_nerr, _old_sys_nerr);
weak_alias (__old_sys_nerr, _old_sys_nerr)
symbol_version (__old_sys_nerr, _sys_nerr, GLIBC_2.0);
symbol_version (_old_sys_nerr, sys_nerr, GLIBC_2.0);
weak_alias (__old_sys_errlist, _old_sys_errlist);
symbol_version (__old_sys_errlist, _sys_errlist, GLIBC_2.0);
symbol_version (_old_sys_errlist, sys_errlist, GLIBC_2.0);

weak_alias (__new_sys_nerr, _new_sys_nerr)
default_symbol_version (__new_sys_nerr, _sys_nerr, GLIBC_2.1);
default_symbol_version (_new_sys_nerr, sys_nerr, GLIBC_2.1);
weak_alias (__new_sys_errlist, _new_sys_errlist)
default_symbol_version (__new_sys_errlist, _sys_errlist, GLIBC_2.1);
default_symbol_version (_new_sys_errlist, sys_errlist, GLIBC_2.1);

#endif
