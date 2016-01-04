/* Machine-dependent ELF dynamic relocation inline functions.  ARM/NaCl version.
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

#ifndef dl_machine_h

/* This is only needed for handling TEXTRELs and NaCl will never
   support TEXTRELs at all.  */
#define CLEAR_CACHE(start, end) __builtin_trap ()

#endif

/* The rest is just machine-specific.
   This #include is outside the #ifndef because the parts of
   dl-machine.h used only by dynamic-link.h are outside the guard.  */
#include <sysdeps/arm/dl-machine.h>

#ifdef dl_machine_h

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */
#undef RTLD_START
#define RTLD_START asm ("\
.text\n\
.globl _start\n\
.type _start, %function\n\
.p2align 4\n\
_start:\n\
	@ r0 has the pointer to the info block (see nacl_startup.h)\n\
	mov r1, sp              @ Save stack base for __libc_stack_end.\n\
	push {r0-r3}            @ Push those, maintaining alignment to 16.\n\
	mov r0, sp              @ Pointer to {info, sp} is argument.\n\
	sfi_bl _dl_start\n\
	pop {r1-r4}             @ Restore stack, getting info block into r1.\n\
	mov lr, #0              @ Return address for noreturn call.\n\
	b _dl_start_user");

#endif
