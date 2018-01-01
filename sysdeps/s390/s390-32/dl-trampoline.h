/* PLT trampolines.  s390 version.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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
 *   v24, v26, v28, v30, v25, v27, v29, v31 : vector parameter registers
 *   24(r15), 28(r15) : PLT arguments PLT1, PLT2
 *   96(r15) : additional stack parameters
 * The normal clobber rules for function calls apply:
 *   r0 - r5 : call clobbered
 *   r6 - r13 :	call saved
 *   r14 : return address (call clobbered)
 *   r15 : stack pointer (call saved)
 *   f4, f6 : call saved
 *   f0 - f3, f5, f7 - f15 : call clobbered
 *   v0 - v3, v5, v7 - v15 : bytes 0-7 overlap with fprs: call clobbered
               bytes 8-15: call clobbered
 *   v4, v6 : bytes 0-7 overlap with f4, f6: call saved
              bytes 8-15: call clobbered
 *   v16 - v31 : call clobbered
 */


	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	cfi_startproc
	.align 16
_dl_runtime_resolve:
	stm    %r2,%r5,32(%r15)		# save registers
	cfi_offset (r2, -64)
	cfi_offset (r3, -60)
	cfi_offset (r4, -56)
	cfi_offset (r5, -52)
	stm    %r14,%r15,48(%r15)
	cfi_offset (r14, -48)
	cfi_offset (r15, -44)
	std    %f0,56(%r15)
	cfi_offset (f0, -40)
	std    %f2,64(%r15)
	cfi_offset (f2, -32)
	lr     %r0,%r15
	lm     %r2,%r3,24(%r15)		# load args saved by PLT
#ifdef RESTORE_VRS
	ahi    %r15,-224		# create stack frame
	cfi_adjust_cfa_offset (224)
	.machine push
	.machine "z13"
	.machinemode "zarch_nohighgprs"
	vstm   %v24,%v31,96(%r15)	# store call-clobbered vr arguments
	cfi_offset (v24, -224)
	cfi_offset (v25, -208)
	cfi_offset (v26, -192)
	cfi_offset (v27, -176)
	cfi_offset (v28, -160)
	cfi_offset (v29, -144)
	cfi_offset (v30, -128)
	cfi_offset (v31, -112)
	.machine pop
#else
	ahi    %r15,-96			# create stack frame
	cfi_adjust_cfa_offset (96)
#endif
	st     %r0,0(%r15)		# write backchain
	basr   %r1,0
0:	l      %r14,1f-0b(%r1)
	bas    %r14,0(%r14,%r1)		# call _dl_fixup
	lr     %r1,%r2			# function addr returned in r2
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	.machinemode "zarch_nohighgprs"
	vlm    %v24,%v31,96(%r15)	# restore vector registers
	.machine pop
	lm     %r14,%r15,272(%r15)# remove stack frame and restore registers
#else
	lm     %r14,%r15,144(%r15)# remove stack frame and restore registers
#endif
	cfi_def_cfa_offset (96)
	ld     %f0,56(%r15)
	ld     %f2,64(%r15)
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
	st     %r12,12(%r15)		# r12 is used as backup of r15
	cfi_offset (r12, -84)
	st     %r14,16(%r15)
	cfi_offset (r14, -80)
	lr     %r12,%r15		# backup stack pointer
	cfi_def_cfa_register (12)
	ahi    %r15,-264		# create stack frame:
					# 96 + sizeof(La_s390_32_regs)
	st     %r12,0(%r15)		# save backchain

	stm    %r2,%r6,96(%r15)		# save registers
	cfi_offset (r2, -264)		# + r6 needed as arg for
	cfi_offset (r3, -260)		#  _dl_profile_fixup
	cfi_offset (r4, -256)
	cfi_offset (r5, -252)
	cfi_offset (r6, -248)
	std    %f0,120(%r15)
	cfi_offset (f0, -240)
	std    %f2,128(%r15)
	cfi_offset (f2, -232)
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	.machinemode "zarch_nohighgprs"
	vstm   %v24,%v31,136(%r15)	# store call-clobbered vr arguments
	cfi_offset (v24, -224)
	cfi_offset (v25, -208)
	cfi_offset (v26, -192)
	cfi_offset (v27, -176)
	cfi_offset (v28, -160)
	cfi_offset (v29, -144)
	cfi_offset (v30, -128)
	cfi_offset (v31, -112)
	.machine pop
#endif

	lm     %r2,%r3,24(%r12)		# load arguments saved by PLT
	lr     %r4,%r14			# return address as third parameter
	basr   %r1,0
0:	l      %r14,6f-0b(%r1)
	la     %r5,96(%r15)		# pointer to struct La_s390_32_regs
	la     %r6,20(%r12)		# long int * framesize
	bas    %r14,0(%r14,%r1)		# call resolver
	lr     %r1,%r2			# function addr returned in r2
	ld     %f0,120(%r15)		# restore call-clobbered arg fprs
	ld     %f2,128(%r15)
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	.machinemode "zarch_nohighgprs"
	vlm    %v24,%v31,136(%r15)	# restore call-clobbered arg vrs
	.machine pop
#endif
	icm    %r0,15,20(%r12)		# load & test framesize
	jnm    2f

	lm     %r2,%r6,96(%r15)		# framesize < 0 means no pltexit call
					# so we can do a tail call without
					# copying the arg overflow area
	lr     %r15,%r12		# remove stack frame
	cfi_def_cfa_register (15)
	l      %r14,16(%r15)		# restore registers
	l      %r12,12(%r15)
	br     %r1			# tail-call to the resolved function

	cfi_def_cfa_register (12)
2:	la     %r4,96(%r15)		# pointer to struct La_s390_32_regs
	st     %r4,32(%r12)
	jz     4f			# framesize == 0 ?
	ahi    %r0,7			# align framesize to 8
	lhi    %r2,-8
	nr     %r0,%r2
	slr    %r15,%r0			# make room for framesize bytes
	st     %r12,0(%r15)		# save backchain
	la     %r2,96(%r15)
	la     %r3,96(%r12)
	srl    %r0,3
3:	mvc    0(8,%r2),0(%r3)		# copy additional parameters
	la     %r2,8(%r2)
	la     %r3,8(%r3)
	brct   %r0,3b
4:	lm     %r2,%r6,0(%r4)		# load register parameters
	basr   %r14,%r1			# call resolved function
	stm    %r2,%r3,40(%r12)		# store return values r2, r3, f0
	std    %f0,48(%r12)		# to struct La_s390_32_retval
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vst    %v24,56(%r12)		# store return value v24
	.machine pop
#endif
	lm     %r2,%r4,24(%r12)		# r2, r3: load arguments saved by PLT
					# r4: pointer to struct La_s390_32_regs
	basr   %r1,0
5:	l      %r14,7f-5b(%r1)
	la     %r5,40(%r12)		# pointer to struct La_s390_32_retval
	bas    %r14,0(%r14,%r1)		# call _dl_call_pltexit

	lr     %r15,%r12		# remove stack frame
	cfi_def_cfa_register (15)
	l      %r14,16(%r15)		# restore registers
	l      %r12,12(%r15)
	lm     %r2,%r3,40(%r15)		# restore return values
	ld     %f0,48(%r15)
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vl    %v24,56(%r15)		# restore return value v24
	.machine pop
#endif
	br     %r14

6:	.long  _dl_profile_fixup - 0b
7:	.long  _dl_call_pltexit - 5b
	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
