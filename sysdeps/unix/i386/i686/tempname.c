/* Copyright (C) 2001 Free Software Foundation, Inc.
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

/* For i686 and up we have a better and faster source of random bits in the
   form of the time stamp counter.  */
#define RANDOM_BITS(Var) \
  if (__builtin_expect (value == UINT64_C (0), 0))			      \
    {									      \
      /* If this is the first time this function is used initialize	      \
	 the variable we accumulate the value in to some somewhat	      \
	 random value.  If we'd not do this programs at startup time	      \
	 might have a reduced set of possible names, at least on slow	      \
	 machines.  */							      \
      struct timeval tv;						      \
      __gettimeofday (&tv, NULL);					      \
      value = ((uint64_t) tv.tv_usec << 16) ^ tv.tv_sec;		      \
    }									      \
  __asm__ __volatile__ ("rdtsc" : "=A" (Var))

#include_next <tempname.c>
