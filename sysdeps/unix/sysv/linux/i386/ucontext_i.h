/* Offsets and other constants needed in the *context() function
   implementation.
   Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define SIG_BLOCK	0
#define SIG_SETMASK	2

/* Offsets of the fields in the ucontext_t structure.  */
#define oLINK		4
#define oSS_SP		8
#define oSS_SIZE	16
#define oGS		20
#define oFS		24
#define oEDI		36
#define oESI		40
#define oEBP		44
#define oESP		48
#define oEBX		52
#define oEDX		56
#define oECX		60
#define oEAX		64
#define oEIP		76
#define oFPREGS		96
#define oSIGMASK	108
#define oFPREGSMEM	236
