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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

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
  if (x == 0 || __sim_disabled_exceptions & x)
    /* Ignore exception.  */
    ;
  else
    raise (SIGFPE);
}
