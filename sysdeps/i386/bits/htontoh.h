/* Copyright (C) 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _NETINET_IN_H
# error "Don't include this file directly, use <netinet/in.h>"
#endif

#if defined __GNUC__ && __GNUC__ >= 2
/* We can use inline assembler instructions to optimize the code.  */

/* To swap the bytes in a word the i486 processors and up provide the
   `bswap' opcode.  On i386 we have to use three instructions.  */
# if !defined __i486__ && !defined __pentium__ && !defined __pentiumpro__

extern __inline u_int32_t
__ntohl (u_int32_t netlong)
{
  register u_int32_t hostlong;

  __asm__ ("rorw $8, %w0; rorl $16, %0; rorw $8, %w0"
	   : "=r" (hostlong)
	   : "0" (netlong));

  return hostlong;
}

# else

extern __inline u_int32_t
__ntohl (u_int32_t netlong)
{
  register u_int32_t hostlong;

  __asm__ ("bswap %0" : "=r" (hostlong) : "0" (netlong));

  return hostlong;
}

# endif

/* For a short word we have a simple solution.  */
extern __inline u_int16_t
__ntohs (u_int16_t netshort)
{
  register u_int16_t hostshort;

  __asm__ ("rorw $8, %w0" : "=r" (hostshort) : "0" (netshort));
}


/* The other direction can be handled with the same functions.  */
extern __inline u_int32_t
__htonl (u_int32_t hostlong)
{
  return __ntohl (hostlong);
}

extern __inline u_int16_t
__htons (u_int16_t hostshort)
{
  return __ntohs (hostshort);
}

#endif /* GNU CC */
