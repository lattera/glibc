/* Install given floating-point environment.
   Copyright (C) 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include <fenv_libc.h>
#include <bp-sym.h>

int
__fesetenv (const fenv_t *envp)
{
  fesetenv_register (*envp);

  /* Success.  */
  return 0;
}
strong_alias (__fesetenv, __old_fesetenv)
symbol_version (BP_SYM (__old_fesetenv), BP_SYM (fesetenv), GLIBC_2.1);
default_symbol_version (BP_SYM (__fesetenv), BP_SYM (fesetenv), GLIBC_2.2);
