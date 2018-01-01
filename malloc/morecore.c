/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#ifndef _MALLOC_INTERNAL
# define _MALLOC_INTERNAL
# include <malloc.h>
#endif

#ifndef __GNU_LIBRARY__
# define __sbrk  sbrk
#endif

#ifdef __GNU_LIBRARY__
/* It is best not to declare this and cast its result on foreign operating
   systems with potentially hostile include files.  */

# include <stddef.h>
# include <stdlib.h>
extern void *__sbrk (ptrdiff_t increment) __THROW;
libc_hidden_proto (__sbrk)
#endif

#ifndef NULL
# define NULL 0
#endif

/* Allocate INCREMENT more bytes of data space,
   and return the start of data space, or NULL on errors.
   If INCREMENT is negative, shrink data space.  */
void *
__default_morecore (ptrdiff_t increment)
{
  void *result = (void *) __sbrk (increment);
  if (result == (void *) -1)
    return NULL;

  return result;
}
libc_hidden_def (__default_morecore)
