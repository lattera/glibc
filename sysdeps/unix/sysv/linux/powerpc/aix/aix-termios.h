/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "aix-types.h"

#define AIX_NCCS 16
struct aixtermios
{
  aixtcflag_t c_iflag;
  aixtcflag_t c_oflag;
  aixtcflag_t c_cflag;
  aixtcflag_t c_lflag;
  aixcc_t c_line;
  aixcc_t c_cc[AIX_NCCS];
  aixspeed_t c_ispeed;
  aixspeed_t c_ospeed;
};

#define AIX_VINTR	0
#define AIX_VQUIT	1
#define AIX_VERASE	2
#define AIX_VKILL	3
#define AIX_VEOF	4
#define AIX_VEOL	5
#define AIX_VEOL2	6
#define AIX_VSTART	7
#define AIX_VSTOP	8
#define AIX_VSUSP	9
#define AIX_VDSUSP	10
#define AIX_VREPRINT	11
#define AIX_VDISCRD	12
#define AIX_VWERSE	13
#define AIX_VLNEXT	14

#define AIX_IUCLC	0x00000800
#define AIX_IXANY	0x00001000
#define AIX_IMAXBE	0x00010000

#define AIX_OLCUC	0x00000002
#define AIX_ONLCR	0x00000004
#define AIX_TABDLY	0x00000c00
#define AIX_TAB0	0x00000000
#define AIX_TAB1	0x00000400
#define AIX_TAB2	0x00000800
#define AIX_TAB3	0x00000c00
#define AIX_BSDLY	0x00001000
#define AIX_BS0		0x00000000
#define AIX_BS1		0x00001000
#define AIX_FFDLY	0x00002000
#define AIX_FF0		0x00000000
#define AIX_FF1		0x00002000
#define AIX_NLDLY	0x00004000
#define AIX_NL0		0x00000000
#define AIX_NL1		0x00004000
#define AIX_VTDLY	0x00008000
#define AIX_VT0		0x00000000
#define AIX_VT1		0x00008000

#define AIX_CBAUD	0x0000000f
#define AIX_CSIZE	0x00000030
#define AIX_CS5		0x00000000
#define AIX_CS6		0x00000010
#define AIX_CS7		0x00000020
#define AIX_CS8		0x00000030
#define AIX_CSTOPB	0x00000040
#define AIX_CREAD	0x00000080
#define AIX_PARENB	0x00000100
#define AIX_PARODD	0x00000200
#define AIX_HUPCL	0x00000400
#define AIX_CLOCAL	0x00000800
