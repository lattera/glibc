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

const char * const __new_sys_siglist[NSIG + 1] =
{
#define init_sig(sig, abbrev, desc)   [sig] desc,
#include "siglist.h"
#undef init_sig
};

const char * const __new_sys_sigabbrev[NSIG + 1] =
{
#define init_sig(sig, abbrev, desc)   [sig] abbrev,
#include "siglist.h"
#undef init_sig
};

#ifdef DO_VERSIONING
strong_alias (__new_sys_siglist, _new_sys_siglist)
default_symbol_version (__new_sys_siglist, _sys_siglist, GLIBC_2.1);
default_symbol_version (_new_sys_siglist, sys_siglist, GLIBC_2.1);
default_symbol_version (__new_sys_sigabbrev, sys_sigabbrev, GLIBC_2.1);
#else
weak_alias(__new_sys_siglist, sys_siglist)
weak_alias(__new_sys_sigabbrev, sys_sigabbrev)
#endif
