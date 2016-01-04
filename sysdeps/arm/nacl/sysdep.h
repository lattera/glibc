/* Assembler macros for ARM/NaCl.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _ARM_NACL_SYSDEP_H
#define _ARM_NACL_SYSDEP_H	1

#ifdef __ASSEMBLER__

# ifndef NO_THUMB
#  define NO_THUMB
# endif
# define ARM_SFI_MACROS		1

/* The compiler driver primes the assembler with a standard set of
   macros that includes sfi_breg and sfi_sp.  The sfi_pld macro is
   redundant with sfi_breg, but libc code uses it so as not to run
   afoul of the assembler's parsing bug in versions prior to 2.23.2.
   NaCl never uses an assembler that has this bug.  */

.macro sfi_pld basereg, offset=#0
	sfi_breg \basereg, pld [\basereg, \offset]
.endm

#endif

#include <sysdeps/arm/sysdep.h>

#ifdef  __ASSEMBLER__

# undef eabi_fnstart
# define eabi_fnstart
# undef eabi_fnend
# define eabi_fnend
# undef eabi_save
# define eabi_save(...)
# undef eabi_cantunwind
# define eabi_cantunwind
# undef eabi_pad
# define eabi_pad(n)

/* NaCl has its own special way of getting the thread pointer.  */
# undef	GET_TLS
# define GET_TLS(tmp)		ldr r0, [r9]

/* Rather than macroizing the code any more, we can just define a few
   mnemonics as macros here.  */
# define bl			sfi_bl
# define bx			sfi_bx
# define blx			sfi_blx
# define bxeq			sfi_bxeq /* Only condition now in use.  */

#endif  /* __ASSEMBLER__ */

#endif  /* sysdep.h */
