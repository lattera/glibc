/* Copyright (C) 1996 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _SYS_SERIAL_H
#define _SYS_SERIAL_H	1
/* Defines for PC AT serial port.  */

/* Serial port addresses and IRQs.  */
#define PORT_0		0x03F8
#define PORT_1		0x02F8
#define IRQ_0		0x04
#define IRQ_1		0x03

/* Definitions for INS8250 / 16550 chips.  */

/* Defined as offsets from the port address (data port).  */
#define DAT	0	/* Receive/transmit data.  */
#define ICR	1	/* Interrupt control register.  */
#define ISR	2	/* Interrupt status register.  */
#define LCR	3	/* Line control register.  */
#define MCR	4	/* Modem control register.  */
#define LSR	5	/* Line status register.  */
#define MSR	6	/* Modem status register.  */
#define DLL	0	/* Divisor latch (lsb).  */
#define DLH	1	/* Divisor latch (msb).  */


/* ICR.  */
#define RIEN	0x01	/* Enable receiver interrupt.  */
#define TIEN	0x02	/* Enable transmitter interrupt.  */
#define SIEN	0x04	/* Enable receiver line status interrupt.  */
#define MIEN	0x08	/* Enable modem status interrupt.  */


/* ISR */
#define FFTMOUT	0x0c	/* Fifo rcvr timeout.  */
#define RSTATUS	0x06	/* Change in receiver line status.  */
#define RxRDY	0x04	/* Receiver data available.  */
#define TxRDY	0x02	/* Transmitter holding register empty.  */
#define MSTATUS	0x00	/* Change in modem status.  */


/* LCR 3
   Number of data bits per received/transmitted character.  */
#define RXLEN	0x03
#define STOP1	0x00
#define STOP2	0x04
#define PAREN	0x08
#define PAREVN	0x10
#define PARMARK	0x20
#define SNDBRK	0x40
#define DLAB	0x80

/* Baud rate definitions.  */
#define ASY9600	12

/* Definitions for character length (data bits) in RXLEN field.  */
#define BITS5	0x00
#define BITS6	0x01
#define BITS7	0x02
#define BITS8	0x03

/* MCR.  */
#define DTR	0x01	/* Bring up DTR.  */
#define RTS	0x02	/* Bring up RTS.  */
#define OUT1	0x04
#define OUT2	0x08
#define LOOP	0x10	/* Put chip into loopback state.  */


/* LSR */
#define RCA	0x01	/* Receive char available.  */
#define OVRRUN	0x02	/* Receive overrun.  */
#define PARERR	0x04	/* Parity error.  */
#define FRMERR	0x08	/* Framing/CRC error.  */
#define BRKDET	0x10	/* Break detected (null char + frame error).  */
#define XHRE	0x20	/* Transmit holding register empty.  */
#define XSRE	0x40	/* Transmit shift register empty.  */


/* MSR */
#define DCTS	0x01	/* CTS has changed state.  */
#define DDSR	0x02	/* DSR has changed state.  */
#define DRI	0x04	/* RI has changed state.  */
#define DDCD	0x08    /* DCD has changed state.  */
#define CTS	0x10	/* State of CTS.  */
#define DSR	0x20	/* State of DSR.  */
#define RI      0x40    /* State of RI.  */
#define DCD     0x80    /* State of DCD.  */


#define DELTAS(x)	((x) & (DCTS | DDSR | DRI | DDCD))
#define STATES(x)	((x) (CTS | DSR | RI | DCD))

#endif /* sys/serial.h */
