/* Copyright (C) 1994, 1997, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.

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

#include <standalone.h>
#include "i386.h"

/*  _Board_Initialize()

This routine initializes the FORCE CPU386 board.  */

void _Console_Initialize (void);

void
_Board_Initialize ()
{
  /*
   *  FORCE documentation incorrectly states that the bus request
   *  level is initialized to 3.  It is actually initialized by
   *  FORCEbug to 0.
   */

  outport_byte (0x00, 0x3f);      /* resets VMEbus request level */

  _Console_Initialize ();
}
