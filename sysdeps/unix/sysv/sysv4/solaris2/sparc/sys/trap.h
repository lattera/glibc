/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef _SYS_TRAP_H
#define _SYS_TRAP_H	1

/* Solaris2 software traps.  */

#define ST_OSYSCALL             0x00
#define ST_BREAKPOINT           0x01
#define ST_DIV0                 0x02
#define ST_FLUSH_WINDOWS        0x03
#define ST_CLEAN_WINDOWS        0x04
#define ST_RANGE_CHECK          0x05
#define ST_FIX_ALIGN            0x06
#define ST_INT_OVERFLOW         0x07
#define ST_SYSCALL              0x08

/* Traps 0x10 through 0x1f are allotted to the user.  */

#endif	/* sys/trap.h */
