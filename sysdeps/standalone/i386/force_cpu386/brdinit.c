/* Copyright (C) 1994 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.
 
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
#include <standalone.h>
#include "__i386.h"

/*  _Board_Initialize()

This routine initializes the FORCE CPU386 board.  */

void DEFUN_VOID(_Console_Initialize);

void 
DEFUN_VOID(_Board_Initialize)
{
  /*
   *  FORCE documentation incorrectly states that the bus request
   *  level is initialized to 3.  It is actually initialized by
   *  FORCEbug to 0.
   */
 
  outport_byte( 0x00, 0x3f );      /* resets VMEbus request level */
 
  _Console_Initialize();
}
