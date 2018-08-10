/* PLT trampolines.  s390x version.
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

/* The PLT stubs will call _dl_runtime_resolve/_dl_runtime_profile
 * with the following linkage:
 *   r2 - r6 : parameter registers
 *   f0, f2, f4, f6 : floating point parameter registers
 *   v24, v26, v28, v30, v25, v27, v29, v31 : vector parameter registers
 *   48(r15), 56(r15) : PLT arguments PLT1, PLT2
 *   160(r15) : additional stack parameters
 * The normal clobber rules for function calls apply:
 *   r0 - r5 : call clobbered
 *   r6 - r13 :	 call saved
 *   r14 : return address (call clobbered)
 *   r15 : stack pointer (call saved)
 *   f0 - f7 : call clobbered
 *   f8 - f15 : call saved
 *   v0 - v7 : bytes 0-7 overlap with f0-f7: call clobbered
               bytes 8-15: call clobbered
 *   v8 - v15 : bytes 0-7 overlap with f8-f15: call saved
                bytes 8-15: call clobbered
 *   v16 - v31 : call clobbered
 */

#define CFA_OFF 160
#define FRAME_OFF CFA_OFF + FRAME_SIZE
#define V24_OFF -288
#define V25_OFF -272
#define V26_OFF -256
#define V27_OFF -240
#define V28_OFF -224
#define V29_OFF -208
#define V30_OFF -192
#define V31_OFF -176
#define PLT1_OFF -112
#define PLT2_OFF -104
#define R2_OFF -96
#define R3_OFF -88
#define R4_OFF -80
#define R5_OFF -72
#define R14_OFF -64
#define R15_OFF -56
#define F0_OFF -48
#define F2_OFF -40
#define F4_OFF -32
#define F6_OFF -24
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	cfi_startproc
	.align 16
_dl_runtime_resolve:
	stmg   %r2,%r5,CFA_OFF+R2_OFF(%r15) # save registers
	cfi_offset (r2, R2_OFF)
	cfi_offset (r3, R3_OFF)
	cfi_offset (r4, R4_OFF)
	cfi_offset (r5, R5_OFF)
	stmg   %r14,%r15,CFA_OFF+R14_OFF(%r15)
	cfi_offset (r14, R14_OFF)
	cfi_offset (r15, R15_OFF)
	std    %f0,CFA_OFF+F0_OFF(%r15)
	cfi_offset (f0, F0_OFF)
	std    %f2,CFA_OFF+F2_OFF(%r15)
	cfi_offset (f2, F2_OFF)
	std    %f4,CFA_OFF+F4_OFF(%r15)
	cfi_offset (f4, F4_OFF)
	std    %f6,CFA_OFF+F6_OFF(%r15)
	cfi_offset (f6, F6_OFF)
	lmg    %r2,%r3,CFA_OFF+PLT1_OFF(%r15) # load args saved by PLT
	lgr    %r0,%r15
#ifdef RESTORE_VRS
# define FRAME_SIZE (CFA_OFF + 128)
	aghi   %r15,-FRAME_SIZE # create stack frame
	cfi_adjust_cfa_offset (FRAME_SIZE)
	.machine push
	.machine "z13"
	vstm   %v24,%v31,FRAME_OFF+V24_OFF(%r15) # save call-clobbered vr args
	cfi_offset (v24, V24_OFF)
	cfi_offset (v25, V25_OFF)
	cfi_offset (v26, V26_OFF)
	cfi_offset (v27, V27_OFF)
	cfi_offset (v28, V28_OFF)
	cfi_offset (v29, V29_OFF)
	cfi_offset (v30, V30_OFF)
	cfi_offset (v31, V31_OFF)
	.machine pop
#else
# define FRAME_SIZE CFA_OFF
	aghi   %r15,-FRAME_SIZE # create stack frame
	cfi_adjust_cfa_offset (FRAME_SIZE)
#endif
	stg    %r0,0(%r15)      # write backchain
	brasl  %r14,_dl_fixup	# call _dl_fixup
	lgr    %r1,%r2		# function addr returned in r2
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vlm    %v24,%v31,FRAME_OFF+V24_OFF(%r15) # restore vector registers
	.machine pop
#endif
	lmg    %r14,%r15,FRAME_OFF+R14_OFF(%r15) # restore frame and registers
#undef FRAME_SIZE
	cfi_def_cfa_offset (CFA_OFF)
	ld     %f0,CFA_OFF+F0_OFF(%r15)
	ld     %f2,CFA_OFF+F2_OFF(%r15)
	ld     %f4,CFA_OFF+F4_OFF(%r15)
	ld     %f6,CFA_OFF+F6_OFF(%r15)
	lmg    %r2,%r5,CFA_OFF+R2_OFF(%r15)
	br     %r1
	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve
#undef V24_OFF
#undef V25_OFF
#undef V26_OFF
#undef V27_OFF
#undef V28_OFF
#undef V29_OFF
#undef V30_OFF
#undef V31_OFF
#undef PLT1_OFF
#undef PLT2_OFF
#undef R2_OFF
#undef R3_OFF
#undef R4_OFF
#undef R5_OFF
#undef R14_OFF
#undef R15_OFF
#undef F0_OFF
#undef F2_OFF
#undef F4_OFF
#undef F6_OFF

#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
	cfi_startproc
	.align 16
_dl_runtime_profile:
	stg    %r12,24(%r15)		# r12 is used as backup of r15
	cfi_offset (r12, -136)
	stg    %r14,32(%r15)
	cfi_offset (r14, -128)
	lgr    %r12,%r15		# backup stack pointer
	cfi_def_cfa_register (12)
	aghi   %r15,-360		# create stack frame:
					# 160 + sizeof(La_s390_64_regs)
	stg    %r12,0(%r15)		# save backchain

	stmg   %r2,%r6,160(%r15)	# save call-clobbered arg regs
	cfi_offset (r2, -360)		# + r6 needed as arg for
	cfi_offset (r3, -352)		#  _dl_profile_fixup
	cfi_offset (r4, -344)
	cfi_offset (r5, -336)
	cfi_offset (r6, -328)
	std    %f0,200(%r15)
	cfi_offset (f0, -320)
	std    %f2,208(%r15)
	cfi_offset (f2, -312)
	std    %f4,216(%r15)
	cfi_offset (f4, -304)
	std    %f6,224(%r15)
	cfi_offset (f6, -296)
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vstm   %v24,%v31,232(%r15)      # store call-clobbered vector arguments
	cfi_offset (v24, -288)
	cfi_offset (v25, -272)
	cfi_offset (v26, -256)
	cfi_offset (v27, -240)
	cfi_offset (v28, -224)
	cfi_offset (v29, -208)
	cfi_offset (v30, -192)
	cfi_offset (v31, -176)
	.machine pop
#endif
	lmg    %r2,%r3,48(%r12)		# load arguments saved by PLT
	lgr    %r4,%r14			# return address as third parameter
	la     %r5,160(%r15)		# pointer to struct La_s390_64_regs
	la     %r6,40(%r12)		# long int * framesize
	brasl  %r14,_dl_profile_fixup	# call resolver
	lgr    %r1,%r2			# function addr returned in r2
	ld     %f0,200(%r15)		# restore call-clobbered arg fprs
	ld     %f2,208(%r15)
	ld     %f4,216(%r15)
	ld     %f6,224(%r15)
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vlm    %v24,%v31,232(%r15)	# restore call-clobbered arg vrs
	.machine pop
#endif
	lg     %r0,40(%r12)		# load framesize
	ltgr   %r0,%r0
	jnm    1f

	lmg    %r2,%r6,160(%r15)	# framesize < 0 means no pltexit call
					# so we can do a tail call without
					# copying the arg overflow area
	lgr    %r15,%r12		# remove stack frame
	cfi_def_cfa_register (15)
	lg     %r14,32(%r15)		# restore registers
	lg     %r12,24(%r15)
	br     %r1			# tail-call to resolved function

	cfi_def_cfa_register (12)
1:	la     %r4,160(%r15)		# pointer to struct La_s390_64_regs
	stg    %r4,64(%r12)
	jz     4f			# framesize == 0 ?
	aghi   %r0,7			# align framesize to 8
	nill   %r0,0xfff8
	slgr   %r15,%r0			# make room for framesize bytes
	stg    %r12,0(%r15)		# save backchain
	la     %r2,160(%r15)
	la     %r3,160(%r12)
	srlg   %r0,%r0,3
3:	mvc    0(8,%r2),0(%r3)		# copy additional parameters
	la     %r2,8(%r2)		# depending on framesize
	la     %r3,8(%r3)
	brctg  %r0,3b
4:	lmg    %r2,%r6,0(%r4)		# restore call-clobbered arg gprs
	basr   %r14,%r1			# call resolved function
	stg    %r2,72(%r12)		# store return values r2, f0
	std    %f0,80(%r12)		# to struct La_s390_64_retval
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vst    %v24,88(%r12)		# store return value v24
	.machine pop
#endif
	lmg    %r2,%r4,48(%r12)		# r2, r3: load arguments saved by PLT
					# r4: pointer to struct La_s390_64_regs
	la     %r5,72(%r12)		# pointer to struct La_s390_64_retval
	brasl  %r14,_dl_call_pltexit

	lgr    %r15,%r12		# remove stack frame
	cfi_def_cfa_register (15)
	lg     %r14,32(%r15)		# restore registers
	lg     %r12,24(%r15)
	lg     %r2,72(%r15)		# restore return values
	ld     %f0,80(%r15)
#ifdef RESTORE_VRS
	.machine push
	.machine "z13"
	vl    %v24,88(%r15)		# restore return value v24
	.machine pop
#endif
	br     %r14			# Jump back to caller

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
