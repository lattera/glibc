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
#include "i386.h"

/* Console IO routines for a FORCE CPU386 board. */

/* Force CPU/386 specific IO addressing
 *
 * The following determines whether Port B or the Console should
 * be used for console I/O.  Setting ONE (and only ONE) of these to 1
 * enables I/O on that port.
 *
 *     PORT A - DUSCC MC68562 Channel A  (*** not supported here ***)
 *     PORT B - DUSCC MC68562 Channel B
 *     PORT C - MFP MC68901 Channel      (*** FORCEbug console ***)
 */

#define PORTB         1               /* use port b as console */
#define PORTC         0               /* use console port as console */

#if ( PORTB == 1 )
#define TX_STATUS     0x1b6           /* DUSCC General Status Register */
#define RX_STATUS     0x1b6           /* DUSCC General Status Register */
#define TX_BUFFER     0x1e0           /* DUSCC Transmitter Channel B */
#define RX_BUFFER     0x1e8           /* DUSCC Receiver Channel B */
#define Is_tx_ready( _status ) ( (_status) & 0x20 )
#define Is_rx_ready( _status ) ( (_status) & 0x10 )
#endif

#if ( PORTC == 1 )
#define TX_STATUS     0x12c           /* MFP Transmit Status Register */
#define RX_STATUS     0x12a           /* MFP Receive Status Register */
#define TX_BUFFER     0x12e           /* MFP Transmitter Channel  */
#define RX_BUFFER     0x12e           /* MFP Receiver Channel  */
#define Is_tx_ready( _status ) ( (_status) & 0x80 )
#define Is_rx_ready( _status ) ( (_status) & 0x80 )
#endif

/* _Console_Initialize

On the Force board the console require some initialization. */

void
_Console_Initialize ()
{
  register unsigned8 ignored;

  /* FORCE technical support mentioned that it may be necessary to
     read the DUSCC RX_BUFFER port four times to remove all junk.
     This code is a little more paranoid.  */

  inport_byte( RX_BUFFER, ignored );
  inport_byte( RX_BUFFER, ignored );
  inport_byte( RX_BUFFER, ignored );
  inport_byte( RX_BUFFER, ignored );
  inport_byte( RX_BUFFER, ignored );
}

/* Miscellaneous support for console IO */

static inline int _Force386_is_rx_ready ()
{
  register unsigned8 status;

  inport_byte( RX_STATUS, status );

  if ( Is_rx_ready( status ) ) return 1;
  else                         return 0;
}

static inline int _Force386_is_tx_ready ()
{
  register unsigned8 status;

  inport_byte( TX_STATUS, status );

  if ( Is_tx_ready( status ) ) return 1;
  else                         return 0;
}


static inline int _Force386_read_data ()
{
  register unsigned8 ch;

#if ( PORTB == 1 )
    /* Force example code resets the Channel B Receiver here.
     * It appears to cause XON's to be lost.
     */

     /* outport_byte( RX_STATUS, 0x10 );  */
#endif

  inport_byte( RX_BUFFER, ch );

  return ch;
}

/* _Console_Putc

This routine transmits a character.  It supports XON/XOFF flow control.  */

#define XON             0x11            /* control-Q */
#define XOFF            0x13            /* control-S */

int
_Console_Putc (ch)
     char ch;
{
  register unsigned8 inch;

  while ( !_Force386_is_tx_ready() );

  while ( _Force386_is_rx_ready() == 1 ) {      /* must be an XOFF */
    inch = _Force386_read_data();
    if ( inch == XOFF )
      do {
        while ( _Force386_is_rx_ready() == 0 );
        inch = _Force386_read_data();
      } while ( inch != XON );
  }

  outport_byte( TX_BUFFER, ch );
  return( 0 );
}

/* _Console_Getc

This routine reads a character from the UART and returns it. */

int
_Console_Getc (poll)
     int poll;
{
  if ( poll ) {
    if ( !_Force386_is_rx_ready() )
      return -1;
    else
      return _Force386_read_data();
  } else {
    while ( !_Force386_is_rx_ready() );
    return _Force386_read_data();
  }
}
