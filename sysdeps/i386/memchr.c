/* memchr (str, ch, n) -- Return pointer to first occurrence of CH in STR less
   than N.
   For Intel 80x86, x>=3.
   Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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

#include <ansidecl.h>
#include <string.h>

#ifdef	__GNUC__

PTR
DEFUN(memchr, (str, c, len),
      CONST PTR str AND int c AND size_t len)
{
  PTR retval;
  asm("cld\n"			/* Search forward.  */
      "testl %1,%1\n"		/* Clear Z flag, to handle LEN == 0.  */
      /* Some old versions of gas need `repne' instead of `repnz'.  */
      "repnz\n"			/* Search for C in al.  */
      "scasb\n"
      "movl %2,%0\n"		/* Set %0 to 0 (without affecting Z flag).  */
      "jnz done\n"		/* Jump if we found nothing equal to C.  */
      "leal -1(%1),%0\n"	/* edi has been incremented.  Return edi-1.  */
      "done:" :
      "=a" (retval), "=D" (str), "=c" (len) :
      "0" (c), "1" (str), "2" (len));
  return retval;
}

#else
#include <sysdeps/generic/memchr.c>
#endif
