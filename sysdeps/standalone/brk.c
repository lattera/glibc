/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <stdlib.h>

PTR __curbrk;
PTR __rorig;
PTR __rlimit;

int
DEFUN(__brk, (inaddr), PTR inaddr)
{

  if ( ( (void *)inaddr > (void *)__rlimit ) || 
                        ( (void *)inaddr < (void *)__rorig ) ) 
    return -1;

  __curbrk = inaddr;
  return 0;
}

/* Initialization Code for Memory Allocation */

PTR __C_heap_start;
int __C_heap_size;
 
#ifdef HAVE_GNU_LD
static
#endif
void
DEFUN(__NONE_set_memvals, (argc, argv, envp),
      int argc AND char **argv AND char **envp)
{
 
  __rorig  = 
  __curbrk = __C_heap_start;
  __rlimit = __curbrk + __C_heap_size;

  (void) &__NONE_set_memvals;    /* Avoid "defined but not used" warning.  */
}
 
#ifdef  HAVE_GNU_LD
 
#include <gnu-stabs.h>

text_set_element (__libc_subinit, __NONE_set_memvals);
 
#endif

