/* Copyright (C) 1991, 1994, 1995, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Ported to standalone by Joel Sherrill jsherril@redstone-emh2.army.mil,
     On-Line Applications Research Corporation.

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

#include <stdlib.h>

void *__curbrk;
void *__rorig;
void *__rlimit;

int
__brk (inaddr)
     void *inaddr;
{

  if ( ( (void *)inaddr > (void *)__rlimit ) ||
                        ( (void *)inaddr < (void *)__rorig ) )
    return -1;

  __curbrk = inaddr;
  return 0;
}

/* Initialization Code for Memory Allocation */

void *__C_heap_start;
int __C_heap_size;

#ifdef HAVE_GNU_LD
static
#endif
void
__NONE_set_memvals (argc, argv, envp)
     int argc;
     char **argv;
     char **envp;
{

  __rorig  =
  __curbrk = __C_heap_start;
  __rlimit = __curbrk + __C_heap_size;

  (void) &__NONE_set_memvals;    /* Avoid "defined but not used" warning.  */
}

#ifdef  HAVE_GNU_LD
text_set_element (__libc_subinit, __NONE_set_memvals);
#endif

weak_alias (__brk, brk)
