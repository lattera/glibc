/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <signal.h>
#include <sizes.h>

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
# define SYS_SIGLIST	__new_sys_siglist
# define SYS_SIGABBREV	__new_sys_sigabbrev
#else
# define SYS_SIGLIST	_sys_siglist
# define SYS_SIGABBREV	_sys_sigabbrev
#endif

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
asm (".data; .globl __old_sys_siglist;  __old_sys_siglist:");
#endif

const char *const SYS_SIGLIST[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] desc,
#include "siglist.h"
#undef init_sig
};

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
asm (".type __old_sys_siglist,%object;.size __old_sys_siglist,"
        OLD_SIGLIST_SIZE_STR "*" PTR_SIZE_STR);

asm (".data; .globl __old_sys_sigabbrev;  __old_sys_sigabbrev:");
#endif

const char *const SYS_SIGABBREV[NSIG] =
{
#define init_sig(sig, abbrev, desc)   [sig] abbrev,
#include "siglist.h"
#undef init_sig
};

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
asm (".type __old_sys_sigabbrev,%object;.size __old_sys_sigabbrev,"
        OLD_SIGLIST_SIZE_STR "*" PTR_SIZE_STR);

extern const char *const *__old_sys_siglist;
extern const char *const *__old_sys_sigabbrev;

strong_alias (__old_sys_siglist, _old_sys_siglist)
symbol_version (__old_sys_siglist, _sys_siglist, GLIBC_2.0);
symbol_version (_old_sys_siglist, sys_siglist, GLIBC_2.0);
symbol_version (__old_sys_sigabbrev, sys_sigabbrev, GLIBC_2.0);

strong_alias (__new_sys_siglist, _new_sys_siglist)
default_symbol_version (__new_sys_siglist, _sys_siglist, GLIBC_2.1);
default_symbol_version (_new_sys_siglist, sys_siglist, GLIBC_2.1);
default_symbol_version (__new_sys_sigabbrev, sys_sigabbrev, GLIBC_2.1);
#else
weak_alias (_sys_siglist, sys_siglist)
weak_alias (_sys_sigabbrev, sys_sigabbrev)
#endif
