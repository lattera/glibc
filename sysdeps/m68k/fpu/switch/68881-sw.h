/* Copyright (C) 1991, 1992, 1997, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef	_68881_SWITCH_H

#define	_68881_SWITCH_H	1
#include <sys/cdefs.h>

/* This is the format of the data at the code label for a function which
   wants to switch depending on whether or not a 68881 is present.

   Initially, `insn' is a `jsr' instruction, and `target' is __68881_switch.
   The first time such a function is called, __68881_switch determines whether
   or not a 68881 is present, and modifies the function accordingly.
   Then `insn' is a `jmp' instruction, and `target' is the value of `fpu'
   if there is 68881, or the value of `soft' if not.  */

struct switch_caller
  {
    unsigned short int insn;	/* The `jsr' or `jmp' instruction.  */
    void *target;		/* The target of the instruction.  */
    void *soft;			/* The address of the soft function.  */
    void *fpu;			/* The address of the 68881 function.  */
  };

/* These are opcodes (values for `insn', above) for `jmp' and `jsr'
   instructions, respectively, to 32-bit absolute addresses.  */
#define	JMP	0x4ef9
#define	JSR	0x4eb9


/* Function to determine whether or not a 68881 is available,
   and modify its caller (which must be a `struct switch_caller', above,
   in data space) to use the appropriate version.  */
extern void __68881_switch (int __dummy) __THROW;


/* Define FUNCTION as a `struct switch_caller' which will call
   `__FUNCTION_68881' if a 68881 is present, and `__FUNCTION_soft' if not.
#define	switching_function(FUNCTION)					      \
  struct switch_caller FUNCTION =					      \
    {									      \
      JSR, (__ptr_t) __68881_switch,					      \
      __CONCAT(__CONCAT(__,FUNCTION),_soft),				      \
      __CONCAT(__CONCAT(__,FUNCTION),_68881)				      \
    }


#endif	/* 68881-switch.h  */
