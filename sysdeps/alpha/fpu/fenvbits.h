/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>, 1997

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

/* This file should never be included directly.  */

#ifndef _FENVBITS_H
#define _FENVBITS_H	1

/* Define the bits representing the exception.

   Note that these are the bit positions as defined by the OSF/1
   ieee_{get,set}_control_word interface and not by the hardware fpcr.

   See the Alpha Architecture Handbook section 4.7.7.3 for details,
   but in summary, trap shadows mean the hardware register can acquire
   extra exception bits so for proper IEEE support the tracking has to
   be done in software -- in this case with kernel support.

   As to why the system call interface isn't in the same format as
   the hardware register, only those crazy folks at DEC can tell you.  */

enum
  {
    FE_INEXACT =	1UL << 21,
#define FE_INEXACT	FE_INEXACT

    FE_UNDERFLOW =	1UL << 20,
#define FE_UNDERFLOW	FE_UNDERFLOW

    FE_OVERFLOW =	1UL << 19,
#define FE_OVERFLOW	FE_OVERFLOW

    FE_DIVBYZERO =	1UL << 18,
#define FE_DIVBYZERO	FE_DIVBYZERO

    FE_INVALID =	1UL << 17,
#define FE_INVALID	FE_INVALID
    
    FE_ALL_EXCEPT =
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)
#define FE_ALL_EXCEPT	FE_ALL_EXCEPT 
  };


/* Alpha chips support all four defined rouding modes. 

   Note that code must be compiled to use dynamic rounding (/d) instructions
   to see these changes.  For gcc this is -mfp-rounding-mode=d; for DEC cc
   this is -fprm d.  The default for both is static rounding to nearest. 

   These are shifted down 58 bits from the hardware fpcr because the 
   functions are declared to take integers.  */

enum
  {
    FE_TOWARDSZERO =	0,
#define FE_TOWARDSZERO	FE_TOWARDSZERO

    FE_DOWNWARD = 	1,
#define FE_DOWNWARD	FE_DOWNWARD

    FE_TONEAREST =	2,
#define FE_TONEAREST	FE_TONEAREST

    FE_UPWARD =		3,
#define FE_UPWARD	FE_UPWARD
  };


/* Type representing exception flags.  */
typedef unsigned long fexcept_t;

/* Type representing floating-point environment.  */
typedef unsigned long fenv_t;

/* If the default argument is used we use this value.  Note that due to
   architecture-specified page mappings, no user-space pointer will ever
   have its two high bits set.  Co-opt one.  */
#define FE_DFL_ENV	((fenv_t *) 0x8800000000000000UL)

#ifdef __USE_GNU
/* Floating-point environment where none of the exceptions are masked.  */
# define FE_NOMASK_ENV	((fenv_t *) 0x880000000000003eUL)
#endif

/* The system calls to talk to the kernel's FP code.  */
extern unsigned long __ieee_get_fp_control(void);
extern void __ieee_set_fp_control(unsigned long);


#endif /* fenvbits.h */
