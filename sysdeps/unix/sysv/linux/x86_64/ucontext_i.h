/* Offsets and other constants needed in the *context() function
   implementation for Linux/x86-64.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

/* Since we cannot include a header to define _NSIG/8, we define it
   here.  */
#define _NSIG8		8

/* Offsets of the fields in the ucontext_t structure.  */
#define oRBP		120
#define oRSP		160
#define oRBX		128
#define oR8		40
#define oR9		48
#define oR12		72
#define oR13		80
#define oR14		88
#define oR15		96
#define oRDI		104
#define oRSI		112
#define oRDX		136
#define oRAX		144
#define oRCX		152
#define oRIP		168
#define oFPREGS		208
#define oSIGMASK	280
#define oFPREGSMEM	408
#define oMXCSR		432
