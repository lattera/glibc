/* Copyright (C) 1994 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
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
#include <unistd.h>
#include <stdlib.h>
#include "__m68020.h"

/* Return control to 135Bug */

void 
DEFUN_VOID(__exit_trap)
{
  set_vbr( 0 );                     /* restore 135Bug vectors */
  asm volatile( "trap   #15"  );    /* trap to 135Bug */
  asm volatile( ".short 0x63" );    /* return to 135Bug (.RETURN) */
  asm volatile( "jmp    main" );    /* restart program */
}

/* The function `_exit' should take a status argument and simply
   terminate program execution, using the low-order 8 bits of the
   given integer as status.  */

__NORETURN void
DEFUN(_exit, (status), int status)
{
  /* status is ignored */

  M68Kvec[ 45 ] = __exit_trap;   /* install exit_trap handler */
  asm volatile( "trap #13" );  /* insures SUPV mode */
}
