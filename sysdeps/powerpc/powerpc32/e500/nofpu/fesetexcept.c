/* Set given exception flags.  e500 version.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <fenv_libc.h>

int
fesetexcept (int excepts)
{
  unsigned long old_spefscr, spefscr;
  int excepts_spe = __fexcepts_to_spe (excepts);

  old_spefscr = fegetenv_register ();
  spefscr = old_spefscr | excepts_spe;
  fesetenv_register (spefscr);

  /* If the state of the "invalid" or "underflow" flag has changed,
     inform the kernel.  */
  if (((spefscr ^ old_spefscr) & (SPEFSCR_FINVS | SPEFSCR_FUNFS)) != 0)
    __fe_note_change ();

  return 0;
}
