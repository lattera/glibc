/* Copyright (C) 1993 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Olaf Flebbe.

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

#include <fpu_control.h>

void
__setfpucw (fpu_control)
     unsigned short fpu_control;
{
  volatile unsigned short cw;

  /* If user supplied _fpu_control, use it ! */
  if (!fpu_control)
  { 
    /* use linux defaults */
    fpu_control = _FPU_DEFAULT;
  }
  /* Get Control Word */
  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );
  
  /* mask in */
  cw &= _FPU_RESERVED;
  cw = cw | (fpu_control & ~_FPU_RESERVED);

  /* set cw */
  __asm__ volatile ("fldcw %0" :: "m" (cw));
}
