/* Copyright (C) 1996-2012 Free Software Foundation, Inc.

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

#include <math.h>

#ifndef FUNC
# error FUNC not defined
#endif

#ifndef TYPE
# define TYPE double
# define REGS "d"
#else
# ifndef REGS
#  error REGS not defined
# endif
#endif

#ifndef INSN
# error INSN not defined
#endif

#define __CONCATX(a,b) __CONCAT(a,b)

TYPE
__CONCATX(__,FUNC) (TYPE x)
{
  TYPE result;
  asm ( INSN "\t%" REGS "0, %" REGS "1" :
	"=w" (result) : "w" (x) );
  return result;
}

weak_alias (__CONCATX(__,FUNC), FUNC)
