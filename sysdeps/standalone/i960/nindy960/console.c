/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
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
#include "i960ca.h"

/* Console IO routines for a NINDY960 board. */

/*
 *  NINDY_IO( ... )
 *
 *  Interface to NINDY.
 */

#define NINDY_INPUT   0
#define NINDY_OUTPUT  1

void ___NINDY_IO_WRAPPER( void )  /* never called */
{
   asm volatile ( "       .text" );
   asm volatile ( "       .align 4" );
   asm volatile ( "       .globl _NINDY_IO" );
   asm volatile ( "_NINDY_IO:" );
   asm volatile ( "        calls   0       /* call console routines */" );
   asm volatile ( "        ret" );
}

/***** !!!! HOW DO I EXFUN NINDY_IO? !!!! *****/

/* _Console_Putc

This routine transmits a character using NINDY.  */

int
_Console_Putc (ch)
     char ch;
{
  NINDY_IO( NINDY_OUTPUT, ch );
  return( 0 );
}

/* _Console_Getc

This routine reads a character from NINDY and returns it. */

int
_Console_Getc (poll)
     int poll;
{
  char ch;

  if ( poll ) {
    /* I don't know how to poll with NINDY */
    return -1;
  } else {
    NINDY_IO( NINDY_INPUT, &ch );
    return ch;
  }
}
