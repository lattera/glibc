/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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

#include <sizes.h>
#include <errlist.h>
#include <shlib-compat.h>

#define SYS_ERRLIST __new_sys_errlist
#define SYS_NERR __new_sys_nerr

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
asm (".data; .globl __old_sys_errlist;  __old_sys_errlist:");
#endif

#include <sysdeps/gnu/errlist.c>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
asm (".type __old_sys_errlist,%object;.size __old_sys_errlist,"
     OLD_ERRLIST_SIZE_STR "*" PTR_SIZE_STR);

extern const char *const *__old_sys_errlist;

const int __old_sys_nerr = OLD_ERRLIST_SIZE;

strong_alias (__old_sys_nerr, _old_sys_nerr);
weak_alias (__old_sys_nerr, _old_sys_nerr)
compat_symbol (libc, __old_sys_nerr, _sys_nerr, GLIBC_2_0);
compat_symbol (libc, _old_sys_nerr, sys_nerr, GLIBC_2_0);
weak_alias (__old_sys_errlist, _old_sys_errlist);
compat_symbol (libc, __old_sys_errlist, _sys_errlist, GLIBC_2_0);
compat_symbol (libc, _old_sys_errlist, sys_errlist, GLIBC_2_0);
#endif

strong_alias (__new_sys_nerr, _new_sys_nerr)
versioned_symbol (libc, __new_sys_nerr, _sys_nerr, GLIBC_2_1);
versioned_symbol (libc, _new_sys_nerr, sys_nerr, GLIBC_2_1);
strong_alias (__new_sys_errlist, _new_sys_errlist)
versioned_symbol (libc, __new_sys_errlist, _sys_errlist, GLIBC_2_1);
versioned_symbol (libc, _new_sys_errlist, sys_errlist, GLIBC_2_1);
