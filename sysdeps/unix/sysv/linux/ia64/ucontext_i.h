/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
     Contributed by David Mosberger-Tang <davidm@hpl.hp.com>.

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

/* Constants shared between setcontext() and getcontext().  Don't
   install this header file.  */

#define SIG_BLOCK	0
#define SIG_UNBLOCK	1
#define SIG_SETMASK	2

#define SC_NAT	0x008
#define SC_BSP	0x048
#define SC_RNAT	0x050
#define SC_UNAT	0x060
#define SC_FPSR	0x068
#define SC_PFS	0x070
#define SC_LC	0x078
#define SC_PR	0x080
#define SC_BR	0x088
#define SC_GR	0x0c8
#define SC_FR	0x1d0
#define SC_MASK	0x9d0


#define rTMP	r16
#define rPOS	r16
#define rCPOS	r17
#define rNAT	r18

#define rB5	r18
#define rB4	r19
#define rB3	r20
#define rB2	r21
#define rB1	r22
#define rB0	r23
#define rRSC	r24
#define rBSP	r25
#define rRNAT	r26
#define rUNAT	r27
#define rFPSR	r28
#define rPFS	r29
#define rLC	r30
#define rPR	r31
