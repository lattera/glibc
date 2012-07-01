/* Software floating-point exception handling emulation.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
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

#include <signal.h>
#include "soft-fp.h"
#include "soft-supp.h"

/* Global to store sticky exceptions.  */
int __sim_exceptions __attribute__ ((nocommon));
libc_hidden_data_def (__sim_exceptions);

/* By default, no exceptions should trap.  */
int __sim_disabled_exceptions = 0xffffffff;
libc_hidden_data_def (__sim_disabled_exceptions);

int __sim_round_mode __attribute__ ((nocommon));
libc_hidden_data_def (__sim_round_mode);

void
__simulate_exceptions (int x)
{
  __sim_exceptions |= x;
  if (x & ~__sim_disabled_exceptions)
    raise (SIGFPE);
}
