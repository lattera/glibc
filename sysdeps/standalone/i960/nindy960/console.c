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
#include "__i960ca.h"

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
DEFUN( _Console_Putc, (ch), char ch )
{
  NINDY_IO( NINDY_OUTPUT, ch );
  return( 0 );
}

/* _Console_Getc

This routine reads a character from NINDY and returns it. */

int
DEFUN( _Console_Getc, (poll), int poll )
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
