/* Copyright (C) 1991, 1995, 1996 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <errno.h>

/* Defined in brk.c.  */
extern void *__curbrk;
extern int __brk (void *addr);

extern int __libc_multiple_libcs;	/* Defined in init-first.c.  */

/* Extend the process's data space by INCREMENT.
   If INCREMENT is negative, shrink data space by - INCREMENT.
   Return start of new space allocated, or -1 for errors.  */
void *
__sbrk (ptrdiff_t increment)
{
  void *oldbrk;

  /* If this is not part of the dynamic library or the library is used
     via dynamic loading in a statically linked program update
     __curbrk from the kernel's brk value.  That way two separate
     instances of __brk and __sbrk can share the heap, returning
     interleaved pieces of it.  */
  if (__curbrk == NULL || __libc_multiple_libcs)
    if (__brk (0) < 0)
      return (void *) -1;

  if (increment == 0)
    return __curbrk;

  oldbrk = __curbrk;
  if (__brk (oldbrk + increment) < 0)
    return (void *) -1;

  return oldbrk;
}

weak_alias (__sbrk, sbrk)
