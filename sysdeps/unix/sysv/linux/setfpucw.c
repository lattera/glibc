/* Set the FPU control word.
Copyright (C) 1996 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "fpu_control.h"

void
__setfpucw (fpu_control_t set)
{
  fpu_control_t cw;

  /* Fetch the current control word.  */
  _FPU_GETCW (cw);

  /* Preserve the reserved bits, and set the rest as the user
     specified (or the default, if the user gave zero).  */
  _FPU_SETCW ((cw & _FPU_RESERVED) | ((set ?: _FPU_DEFAULT) & ~_FPU_RESERVED));
}

/* The startup code in init-first.c calls __setfpucw (__fpu_control).  */

fpu_control_t __fpu_control;
