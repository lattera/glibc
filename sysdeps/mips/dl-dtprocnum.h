/* Configuration of lookup functions.  MIPS version.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

/* Until elf/elf.h in glibc is updated.  */
#ifndef STO_MIPS_PLT
# undef DT_MIPS_NUM
# define DT_MIPS_NUM 0x35
#endif

/* Number of extra dynamic section entries for this architecture.  By
   default there are none.  */
#define DT_THISPROCNUM	DT_MIPS_NUM
