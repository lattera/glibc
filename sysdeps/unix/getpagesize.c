/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stddef.h>
#include <sys/param.h>

/* Return the system page size.  */
size_t
DEFUN_VOID(__getpagesize)
{
#ifdef	EXEC_PAGESIZE
  return EXEC_PAGESIZE;
#else	/* No EXEC_PAGESIZE.  */
#ifdef	NBPG
#ifndef	CLSIZE
#define	CLSIZE	1
#endif	/* No CLSIZE.  */
  return NBPG * CLSIZE;
#else	/* No NBPG.  */
  return NBPC;
#endif	/* NBPG.  */
#endif	/* EXEC_PAGESIZE.  */
}

weak_alias (__getpagesize, getpagesize)
