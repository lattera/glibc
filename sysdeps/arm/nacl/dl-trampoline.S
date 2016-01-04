/* PLT trampolines.  ARM/NaCl version.
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

#include <sysdep.h>

	.syntax unified
	.text

@ Change &GOT[n+3] into 8*n.  Note relocs are 8 bytes each.
.macro compute_reloc_arg pltgot, got2
	sub r1, \pltgot, \got2	@ r1 = &GOT[n+3] - &GOT[2] = 4*(n-1)
	sub r1, r1, #4		@ r1 = 4*n
	add r1, r1, r1		@ r1 *= 2 = 8*n
.endm

	CFI_SECTIONS
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, %function
	.p2align 4
_dl_runtime_resolve:
	cfi_startproc
	cfi_adjust_cfa_offset (8)

	@ We get called with:
	@ 	lr contains the return address from this call
	@	stack[1] contains &GOT[n+3] (pointer to function)
	@	stack[0] points to &GOT[2]

	ldr ip, [sp]		@ ip gets &GOT[2]

	@ Save the argument registers and the return address.
	@ r4 doesn't need to be saved, but it makes the total
	@ adjustment to sp (including the two words pushed by
	@ the PLT code) an even eight words, so sp stays aligned.
	push {r0-r4, lr}
	cfi_adjust_cfa_offset (24)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r2, 8)
	cfi_rel_offset (r3, 12)
	cfi_rel_offset (r4, 16)
	cfi_rel_offset (lr, 20)

	ldr r1, [sp, #28]	@ r1 gets &GOT[n+3]

	@ Get the 'struct link_map *' for the first argument to _dl_fixup.
	sfi_breg ip, ldr r0, [\B, #-4]

	@ Get the reloc offset for the second argument to _dl_fixup.
	compute_reloc_arg r1, ip

	@ This does the real work, and returns the real call target.
	sfi_bl _dl_fixup
	mov ip, r0

	@ Restore the saved registers.
	pop {r0-r4, lr}
	cfi_adjust_cfa_offset (-24)
	cfi_restore (r0)
	cfi_restore (r1)
	cfi_restore (r2)
	cfi_restore (r3)
	cfi_restore (r4)
	cfi_restore (lr)

	@ Now compensate for the two words pushed by the PLT code.
	sfi_sp add sp, #8
	cfi_adjust_cfa_offset (-8)

	@ Finally, jump to the newfound call target.
	sfi_bx ip

	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve

#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, #function
	.p2align 4
_dl_runtime_profile:
	cfi_startproc
	cfi_adjust_cfa_offset (8)

	@ We get called with:
	@ 	lr contains the return address from this call
	@	stack[1] contains &GOT[n+3] (pointer to function)
	@	stack[0] points to &GOT[2]

	@ Stack layout:
	@ sp + 204		framesize returned from pltenter
	@ sp + 12		La_arm_regs
	@ sp + 4		Saved two arguments to _dl_profile_fixup
	@ sp + 0		outgoing argument to _dl_profile_fixup
	@ For now, we only save the general purpose registers.
# define PLTEXIT_ARGS		4
# define LA_ARM_REGS		(PLTEXIT_ARGS + 8)
# define LA_ARM_REGS_SIZE	(4 * (4 + 1 + 1 + 42))
# define PLTENTER_FRAMESIZE	(LA_ARM_REGS + LA_ARM_REGS_SIZE)
# define FRAMESIZE		(((PLTENTER_FRAMESIZE + 4) + 15) & -16)

	@ The NaCl ABI requires that sp be aligned to 16 bytes at call
	@ sites.  Assuming that was met on entry to the PLT, sp is
	@ now exactly 8 bytes misaligned.
	sfi_sp sub sp, #(FRAMESIZE - 8)
	cfi_def_cfa_offset (FRAMESIZE)

	@ Store the argument registers in La_arm_regs.
	strd r0, r1, [sp, #LA_ARM_REGS]
	cfi_offset (r0, LA_ARM_REGS + 0)
	cfi_offset (r1, LA_ARM_REGS + 4)
	strd r2, r3, [sp, #(LA_ARM_REGS + 8)]
	cfi_offset (r2, LA_ARM_REGS + 8)
	cfi_offset (r3, LA_ARM_REGS + 12)

	ldr ip, [sp, #(FRAMESIZE - 8)]		@ ip gets &GOT[2]
	ldr r3, [sp, #(FRAMESIZE - 4)]		@ r3 gets &GOT[n+3]

	@ Recover the incoming sp and lr and save those in La_arm_regs.
	add r0, sp, #FRAMESIZE
	mov r1, lr
	strd r0, r1, [sp, #(LA_ARM_REGS + 16)]
	cfi_offset (sp, LA_ARM_REGS + 16)
	cfi_offset (lr, LA_ARM_REGS + 20)

	@ Get the 'struct link_map *' for the first arg to _dl_profile_fixup.
	sfi_breg ip, ldr r0, [\B, #-4]

	@ Get the reloc offset for the second argument to _dl_profile_fixup.
	compute_reloc_arg r3, ip

	@ The third argument is the original return address, still in lr.
	mov r2, lr

	@ Compute the fourth argument, the La_arm_regs pointer.
	add r3, sp, #PLTEXIT_ARGS

	@ Compute the fifth argument, the address of the 'framesize'
	@ out parameter, and store it at the top of the stack.
	add ip, sp, #PLTENTER_FRAMESIZE
	str ip, [sp]

	@ Save away the first two arguments, which we will need
	@ again for _dl_call_pltexit, below.
	strd r0, r1, [sp, #PLTEXIT_ARGS]

	@ This does the real work, and returns the real call target.
	sfi_bl _dl_profile_fixup

	@ The address to call is now in r0.

	@ Check whether we're wrapping this function,
	@ i.e. if the framesize out parameter is >= 0.
	ldr	ip, [sp, #PLTENTER_FRAMESIZE]
	cmp	ip, #0
	bge	1f
	cfi_remember_state

	@ Save _dl_profile_fixup's return value: the real call target.
	mov ip, r0

	@ Restore the registers from the La_arm_regs (perhaps as modified
	@ by audit modules' pltenter functions).
	add r1, sp, #LA_ARM_REGS
	sfi_sp sfi_breg r1, ldmia \B, {r0-r3, sp, lr}
	cfi_def_cfa_offset (0)
	cfi_restore (r0)
	cfi_restore (r1)
	cfi_restore (r2)
	cfi_restore (r3)
	cfi_restore (sp)
	cfi_restore (lr)

	@ Finally, jump to the newfound call target.
	sfi_bx ip

1:	cfi_restore_state
	@ The new frame size is in ip.

	@ Save the fp in the stack slot previously used for the fifth
	@ argument to _dl_profile_fixup.
	str fp, [sp]
	cfi_offset (fp, 0)

	@ Save the result of _dl_profile_fixup, the real call target.
	@ We'll reuse the stack slot just used for the 'framesize'
	@ out parameter to _dl_profile_fixup.
	str r0, [sp, #PLTENTER_FRAMESIZE]

	@ Stack layout:
	@ fp + 264		call target
	@ fp + 72		La_arm_regs
	@ fp + 68		Saved two arguments to _dl_profile_fixup
	@ fp + 64		saved fp
	@ fp + 0		La_arm_retval
	@ sp..fp		copied incoming stack space (plus alignment)
	@ For now, we only save the general purpose registers.
# define FP_LA_ARM_RETVAL	0
# define LA_ARM_RETVAL_SIZE	(4 * (4 + 12))
# define FP_SAVED_FP		LA_ARM_RETVAL_SIZE
# define FP_PLTEXIT_ARGS	(FP_SAVED_FP + 4)
# define FP_LA_ARM_REGS		(FP_PLTEXIT_ARGS + 8)
# define FP_CALL_TARGET		(FP_LA_ARM_REGS + LA_ARM_REGS_SIZE)
# define FP_FRAMESIZE		(FP_CALL_TARGET + 4)

	sub fp, sp, #(FP_FRAMESIZE - FRAMESIZE)
	cfi_def_cfa (fp, FP_FRAMESIZE)

	sub r1, fp, ip
	@ This doesn't need sfi_sp because we just include the
	@ sandboxing mask along with the alignment mask.
	bic sp, r1, #0xc000000f

	@ Copy the stack arguments.  The audit modules' pltenter
	@ function(s) decided how much needs to be copied.
	@ Load the sp as modified by pltenter functions, rather
	@ than what we think the incoming sp was (fp + FP_FRAMESIZE).
	sfi_breg fp, ldr r1, [\B, #(FP_LA_ARM_REGS + 16)]
	mov r0, sp
	mov r2, ip
	sfi_bl memcpy

	@ Load up the arguments from La_arm_regs and call the user's function.
	sfi_breg fp, ldr ip, [\B, #FP_CALL_TARGET]
	sfi_breg fp, ldrd r0, r1, [\B, #FP_LA_ARM_REGS]
	sfi_breg fp, ldrd r2, r3, [\B, #(FP_LA_ARM_REGS + 8)]
	sfi_blx ip

	@ Stash the return value registers in La_arm_retval.
	sfi_breg fp, strd r0, r1, [\B, #FP_LA_ARM_RETVAL]
	sfi_breg fp, strd r2, r3, [\B, #(FP_LA_ARM_RETVAL + 8)]

	@ Call pltexit.  We saved the first two arguments earlier--they
	@ are the same ones passed to _dl_profile_fixup.  The latter two
	@ arguments are La_arm_regs and La_arm_retval blocks, respectively.
	sfi_breg fp, ldrd r0, r1, [\B, #FP_PLTEXIT_ARGS]
	add r2, fp, #FP_LA_ARM_REGS
	add r3, fp, #FP_LA_ARM_RETVAL
	sfi_bl _dl_call_pltexit

	@ Reload the saved return value registers for the caller.
	sfi_breg fp, ldrd r0, r1, [\B, #FP_LA_ARM_RETVAL]
	sfi_breg fp, ldrd r2, r3, [\B, #(FP_LA_ARM_RETVAL + 8)]

	@ Unwind the frame.
	sfi_sp mov sp, fp
	cfi_def_cfa_register (sp)
	ldr fp, [sp, #FP_SAVED_FP]
	cfi_restore (fp)
	@ Reload the lr and sp values from La_arm_regs, where they
	@ might have been modified by pltenter functions, rather than
	@ computing what we think the incoming value was.
	ldr lr, [sp, #(FP_LA_ARM_REGS + 20)]
	cfi_restore (lr)
	sfi_sp ldr sp, [sp, #(FP_LA_ARM_REGS + 16)]
	cfi_def_cfa_offset (0)

	@ Finally, return to the caller.
	sfi_bx lr

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
	.previous
