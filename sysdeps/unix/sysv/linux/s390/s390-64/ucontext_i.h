/* Copyright (C) 2001 Free Software Foundation, Inc.
     Contributed by David Mosberger-Tang <davidm@hpl.hp.com>.

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
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Constants shared between setcontext() and getcontext().  Don't
   install this header file.  */

#define SIG_BLOCK	0
#define SIG_UNBLOCK	1
#define SIG_SETMASK	2

#define SC_FLGS 0x000
#define SC_LINK 0x008
#define SC_MASK 0x010
#define SC_STCK 0x090
#define SC_VERS 0x0a8
#define SC_GPRS 0x0b0
#define SC_FPC  0x188
#define SC_FPRS 0x190

