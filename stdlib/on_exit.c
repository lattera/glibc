/* Copyright (C) 1991 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stdlib.h>
#include "exit.h"

/* Register a function to be called by exit.  */
int
DEFUN(on_exit, (func, arg),
      void EXFUN((*func), (int status, PTR arg)) AND PTR arg)
{
  struct exit_function *new = __new_exitfn();

  if (new == NULL)
    return -1;

  new->flavor = ef_on;
  new->func.on.fn = func;
  new->func.on.arg = arg;
  return 0;
}
