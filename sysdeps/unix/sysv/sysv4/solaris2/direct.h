/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

#ifndef	   _BSDDIR_H
#define	   _BSDDIR_H	1

#include <limits.h>

/* This is the Solaris direct; it's the same as that in
   sysdeps/unix/sysv/sysv4/direct.h, but it uses the length given by d_namlen,
   since we can't reliably use tyhe sysv4/direct.h method of computing
   the length.  */

struct direct
  {
    unsigned long int d_fileno;
    long int d_off;
    unsigned short int d_reclen;
    char d_name[NAME_MAX + 1];
  };

#define D_NAMLEN(d) (strlen ((d)->d_name))

#endif
