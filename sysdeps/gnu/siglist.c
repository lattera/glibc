/* Define list of all signal numbers and their names.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <signal.h>
#include <libintl.h>
#include <shlib-compat.h>

#include <bits/wordsize.h>
#if	__WORDSIZE == 32
#define PTR_SIZE_STR "4"
#elif	__WORDSIZE == 64
#define PTR_SIZE_STR "8"
#else
#error	unexpected wordsize __WORDSIZE
#endif


#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
asm (".data\n\t.globl __old_sys_siglist\n__old_sys_siglist:");
#endif

const char *const __new_sys_siglist[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] desc,
#include <siglist.h>
#undef init_sig
};

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
asm (".type __old_sys_siglist,@object\n\t.size __old_sys_siglist,"
        OLD_SIGLIST_SIZE_STR "*" PTR_SIZE_STR);

asm (".data\n\t.globl __old_sys_sigabbrev\n__old_sys_sigabbrev:");
#endif

const char *const __new_sys_sigabbrev[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] abbrev,
#include <siglist.h>
#undef init_sig
};

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
asm (".type __old_sys_sigabbrev,@object\n\t.size __old_sys_sigabbrev,"
        OLD_SIGLIST_SIZE_STR "*" PTR_SIZE_STR);

extern const char *const *__old_sys_siglist;
extern const char *const *__old_sys_sigabbrev;

strong_alias (__old_sys_siglist, _old_sys_siglist)
compat_symbol (libc, __old_sys_siglist, _sys_siglist, GLIBC_2_0);
compat_symbol (libc, _old_sys_siglist, sys_siglist, GLIBC_2_0);
compat_symbol (libc, __old_sys_sigabbrev, sys_sigabbrev, GLIBC_2_0);
#endif

strong_alias (__new_sys_siglist, _new_sys_siglist)
versioned_symbol (libc, __new_sys_siglist, _sys_siglist, GLIBC_2_1);
versioned_symbol (libc, _new_sys_siglist, sys_siglist, GLIBC_2_1);
versioned_symbol (libc, __new_sys_sigabbrev, sys_sigabbrev, GLIBC_2_1);
