/* Copyright (C) 1994 Free Software Foundation, Inc.
   Ported to standalone by Joel Sherrill jsherril@redstone-emh2.army.mil,
     On-Line Applications Research Corporation.
 
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *  This is the file descriptor used by the no OS implementation
 *  of __open, __read, __write, and __close.
 */

#ifndef __FILEDESC_h
#define __FILEDESC_h

#include <stdio_lim.h>

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
