/* Internal support stuff for complete soft float.
   Copyright (C) 2002-2013 Free Software Foundation, Inc.
   Contributed by Aldy Hernandez <aldyh@redhat.com>, 2002.
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

#if defined __NO_FPRS__ && !defined _SOFT_FLOAT

# include <fenv_libc.h>

#else

# include <fenv.h>

typedef union
{
  fenv_t fenv;
  unsigned int l[2];
} fenv_union_t;

#endif

/* FIXME: these variables should be thread specific (see bugzilla bug
   15483) and ideally preserved across signal handlers, like hardware
   FP status words, but the latter is quite difficult to accomplish in
   userland.  */

extern int __sim_exceptions;
libc_hidden_proto (__sim_exceptions);
extern int __sim_disabled_exceptions;
libc_hidden_proto (__sim_disabled_exceptions);
extern int __sim_round_mode;
libc_hidden_proto (__sim_round_mode);

extern void __simulate_exceptions (int x) attribute_hidden;
