/* PLT trampolines.  s390 version.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */

/* The PLT stubs will call _dl_runtime_resolve/_dl_runtime_profile
 * with the following linkage:
 *   r2 - r6 : parameter registers
 *   f0, f2 : floating point parameter registers
 *   24(r15), 28(r15) : PLT arguments PLT1, PLT2
 *   96(r15) : additional stack parameters
 * The normal clobber rules for function calls apply:
 *   r0 - r5 : call clobbered
 *   r6 - r13 :	 call saved
 *   r14 : return address (call clobbered)
 *   r15 : stack pointer (call saved)
 *   f4, f6 : call saved
 *   f0 - f3, f5, f7 - f15 : call clobbered
 */

#include <sysdep.h>

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	cfi_startproc
	.align 16
_dl_runtime_resolve:
	stm    %r2,%r5,32(%r15)		# save registers
	st     %r14,8(%r15)
	cfi_offset (r14, -88)
	lr     %r0,%r15			# create stack frame
	ahi    %r15,-96
	cfi_adjust_cfa_offset (96)
	st     0,0(%r15)
	lm     %r2,%r3,120(%r15)	# load args saved by PLT
	basr   %r1,0
0:	l      %r14,1f-0b(%r1)
	bas    %r14,0(%r14,%r1)		# call resolver
	lr     %r1,%r2			# function addr returned in r2
	ahi    %r15,96			# remove stack frame
	cfi_adjust_cfa_offset (-96)
	l      %r14,8(15)		# restore registers
	lm     %r2,%r5,32(%r15)
	br     %r1
1:	.long  _dl_fixup - 0b
	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve


#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
	cfi_startproc
	.align 16
_dl_runtime_profile:
	stm    %r2,%r6,32(%r15)		# save registers
	std    %f0,56(%r15)
	std    %f2,64(%r15)
	st     %r6,8(%r15)
	st     %r12,12(%r15)
	st     %r14,16(%r15)
	cfi_offset (r6, -64)
	cfi_offset (f0, -40)
	cfi_offset (f2, -32)
	cfi_offset (r12, -84)
	cfi_offset (r14, -80)
	lr     %r12,%r15		# create stack frame
	cfi_def_cfa_register (12)
	ahi    %r15,-96
	st     %r12,0(%r15)
	lm     %r2,%r3,24(%r12)		# load arguments saved by PLT
	lr     %r4,%r14			# return address as third parameter
	basr   %r1,0
0:	l      %r14,6f-0b(%r1)
	la     %r5,32(%r12)		# pointer to struct La_s390_32_regs
	la     %r6,20(%r12)		# long int * framesize
	bas    %r14,0(%r14,%r1)		# call resolver
	lr     %r1,%r2			# function addr returned in r2
	icm    %r0,15,20(%r12)		# load & test framesize
	jnm    2f
	lm     %r2,%r6,32(%r12)
	ld     %f0,56(%r12)
	ld     %f2,64(%r12)
	basr   %r14,%r1			# call resolved function
1:	lr     %r15,%r12		# remove stack frame
	cfi_def_cfa_register (15)
	l      %r14,16(%r15)		# restore registers
	l      %r12,12(%r15)
	l      %r6,8(%r15)
	br     %r14
	cfi_def_cfa_register (12)
2:	jz     4f			# framesize == 0 ?
	ahi    %r0,7			# align framesize to 8
	lhi    %r2,-8
	nr     %r0,%r2
	slr    %r15,%r0			# make room for framesize bytes
	st     %r12,0(%r15)
	la     %r2,96(%r15)
	la     %r3,96(%r12)
	srl    %r0,3
3:	mvc    0(8,%r2),0(%r3)		# copy additional parameters
	la     %r2,8(%r2)
	la     %r3,8(%r3)
	brct   %r0,3b
4:	lm     %r2,%r6,32(%r12)		# load register parameters
	ld     %f0,56(%r12)
	ld     %f2,64(%r12)
	basr   %r14,%r1			# call resolved function
	stm    %r2,%r3,72(%r12)
	std    %f0,80(%r12)
	lm     %r2,%r3,24(%r12)		# load arguments saved by PLT
	basr   %r1,0
5:	l      %r14,7f-5b(%r1)
	la     %r4,32(%r12)		# pointer to struct La_s390_32_regs
	la     %r5,72(%r12)		# pointer to struct La_s390_32_retval
	basr   %r14,%r1			# call _dl_call_pltexit
	j      1b
6:	.long  _dl_profile_fixup - 0b
7:	.long  _dl_call_pltexit - 5b
	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
