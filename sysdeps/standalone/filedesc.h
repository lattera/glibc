/* Copyright (C) 1994, 1997, 1998 Free Software Foundation, Inc.
   Ported to standalone by Joel Sherrill jsherril@redstone-emh2.army.mil,
     On-Line Applications Research Corporation.
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

/*
 *  This is the file descriptor used by the no OS implementation
 *  of __open, __read, __write, and __close.
 */

#ifndef __FILEDESC_h
#define __FILEDESC_h

#define __need_FOPEN_MAX
#include <bits/stdio_lim.h>

#ifndef __DECLARE_FILE_DESCRIPTORS__
#define FILEDESC_EXTERN extern
#else
#define FILEDESC_EXTERN
#endif

typedef struct {
  int  in_use;         /* 1 if in use, 0 otherwise */
  int  flags;          /* Flags from open */
}   __no_os_file_descriptor;

#define __FD_Is_valid( _fd ) \
  ( (_fd) >= 0 && (_fd) < FOPEN_MAX )

FILEDESC_EXTERN __no_os_file_descriptor __FD_Table[ FOPEN_MAX ];

#endif
