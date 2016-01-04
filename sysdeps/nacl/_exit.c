/* _exit -- low-level program termination.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <stdlib.h>
#include <nacl-interfaces.h>

void
_exit (int status)
{
  __nacl_irt_basic.exit (status);

  /* That never returns unless something is severely and unrecoverably wrong.
     If it ever does, try to make sure we crash.  */
  while (1)
    __builtin_trap ();
}
libc_hidden_def (_exit)
rtld_hidden_def (_exit)
weak_alias (_exit, _Exit)
