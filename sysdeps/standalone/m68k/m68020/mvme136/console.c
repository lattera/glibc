/* Copyright (C) 1994, 1996 Free Software Foundation, Inc.
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
#include "m68020.h"

/* Console IO routines for a Motorola MVME135/MVME136 board.

They currently use the B port.  It should be possible to
use the A port by filling in the reset of the chip structure,
adding an ifdef for PORTA/PORTB, and switching the addresses,
and maybe the macros based on the macro. */

/* M68681 DUART chip register structures and constants */

typedef struct {
  volatile unsigned char fill1[ 5 ];     /* channel A regs ( not used ) */
  volatile unsigned char isr;            /* interrupt status reg */
  volatile unsigned char fill2[ 2 ];     /* counter regs (not used) */
  volatile unsigned char mr1mr2b;        /* MR1B and MR2B regs */
  volatile unsigned char srb;            /* status reg channel B */
  volatile unsigned char fill3;          /* do not access */
  volatile unsigned char rbb;            /* receive buffer channel B */
  volatile unsigned char ivr;            /* interrupt vector register */
} r_m681_info;

typedef struct {
  volatile unsigned char fill1[ 4 ];     /* channel A regs (not used) */
  volatile unsigned char acr;            /* auxillary control reg */
  volatile unsigned char imr;            /* interrupt mask reg */
  volatile unsigned char fill2[ 2 ];     /* counter regs (not used) */
  volatile unsigned char mr1mr2b;        /* MR1B and MR2B regs */
  volatile unsigned char csrb;           /* clock select reg */
  volatile unsigned char crb;            /* command reg */
  volatile unsigned char tbb;            /* transmit buffer channel B */
  volatile unsigned char ivr;            /* interrupt vector register */
} w_m681_info;

#define RD_M68681     ((r_m681_info *)0xfffb0040)   /* ptr to the M68681 */
#define WR_M68681     ((w_m681_info *)0xfffb0040)   /* ptr to the M68681 */
#define RXRDYB        0x01               /* status reg recv ready mask */
#define TXRDYB        0x04               /* status reg trans ready mask */

/* _Console_Putc

This routine transmits a character out the M68681.  It supports
XON/XOFF flow control.  */

#define XON             0x11            /* control-Q */
#define XOFF            0x13            /* control-S */

int
_Console_Putc (ch)
     char ch;
{
  while ( ! (RD_M68681->srb & TXRDYB) ) ;
  while ( RD_M68681->srb & RXRDYB )        /* must be an XOFF */
    if ( RD_M68681->rbb == XOFF )
      do {
        while ( ! (RD_M68681->srb & RXRDYB) ) ;
      } while ( RD_M68681->rbb != XON );

  WR_M68681->tbb = ch;
  return( 0 );
}

/* _Console_Getc

This routine reads a character from the UART and returns it. */

int
_Console_Getc (poll)
     int poll;
{
  if ( poll ) {
    if ( !(RD_M68681->srb & RXRDYB) )
      return -1;
    else
      return RD_M68681->rbb;
  } else {
    while ( !(RD_M68681->srb & RXRDYB) );
    return RD_M68681->rbb;
  }
}
