/* PLT trampolines.  ia64 version.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

/*
   This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns. `fixup()' takes two
   arguments, however profile_fixup() takes three.

   The ABI specifies that we will never see more than 8 input
   registers to a function call, thus it is safe to simply allocate
   those, and simpler than playing stack games.  */

ENTRY(_dl_runtime_resolve)
	{ .mmi
	  .prologue
	  .save ar.pfs, r40
	  alloc loc0 = ar.pfs, 8, 6, 3, 0
	  adds r2 = -144, r12
	  adds r3 = -128, r12
	}
	{ .mii
	  .fframe 160
	  adds r12 = -160, r12
	  .save rp, r41
	  mov loc1 = b0
	  .body
	  mov out2 = b0		/* needed by fixup_profile */
	  ;;
	}
	{ .mfb
	  mov loc2 = r8		/* preserve struct value register */
	  nop.f 0
	  nop.b 0
	}
	{ .mii
	  mov loc3 = r9		/* preserve language specific register */
	  mov loc4 = r10	/* preserve language specific register */
	  mov loc5 = r11	/* preserve language specific register */
	}
	{ .mmi
	  stf.spill [r2] = f8, 32
	  stf.spill [r3] = f9, 32
	  mov out0 = r16
	  ;;
	}
	{ .mmi
	  stf.spill [r2] = f10, 32
	  stf.spill [r3] = f11, 32
	  shl out1 = r15, 4
	  ;;
	}
	{ .mmi
	  stf.spill [r2] = f12, 32
	  stf.spill [r3] = f13, 32
	  shladd out1 = r15, 3, out1
	  ;;
	}
	{ .mmb
	  stf.spill [r2] = f14
	  stf.spill [r3] = f15
	  br.call.sptk.many b0 = fixup
	}
	{ .mii
	  ld8 r9 = [ret0], 8
	  adds r2 = 16, r12
	  adds r3 = 32, r12
	  ;;
	}
	{ .mmi
	  ldf.fill f8 = [r2], 32
	  ldf.fill f9 = [r3], 32
	  mov b0 = loc1
	  ;;
	}
	{ .mmi
	  ldf.fill f10 = [r2], 32
	  ldf.fill f11 = [r3], 32
	  mov b6 = r9
	  ;;
	}
	{ .mmi
	  ldf.fill f12 = [r2], 32
	  ldf.fill f13 = [r3], 32
	  mov ar.pfs = loc0
	  ;;
	}
	{ .mmi
	  ldf.fill f14 = [r2], 32
	  ldf.fill f15 = [r3], 32
	  .restore sp		/* pop the unwind frame state */
	  adds r12 = 160, r12
	  ;;
	}
	{ .mii
	  mov r9 = loc3		/* restore language specific register */
	  mov r10 = loc4	/* restore language specific register */
	  mov r11 = loc5	/* restore language specific register */
	}
	{ .mii
	  ld8 gp = [ret0]
	  mov r8 = loc2		/* restore struct value register */
	  ;;
	}
	/* An alloc is needed for the break system call to work.
	   We don't care about the old value of the pfs register.  */
	{ .mmb
	  .prologue
	  .body
	  alloc r2 = ar.pfs, 0, 0, 8, 0
	  br.sptk.many b6
	  ;;
	}
END (_dl_runtime_resolve)


ENTRY(_dl_runtime_profile)
	{ .mmi
	  .prologue
	  .save ar.pfs, r40
	  alloc loc0 = ar.pfs, 8, 6, 3, 0
	  adds r2 = -144, r12
	  adds r3 = -128, r12
	}
	{ .mii
	  .fframe 160
	  adds r12 = -160, r12
	  .save rp, r41
	  mov loc1 = b0
	  .body
	  mov out2 = b0		/* needed by fixup_profile */
	  ;;
	}
	{ .mfb
	  mov loc2 = r8		/* preserve struct value register */
	  nop.f 0
	  nop.b 0
	}
	{ .mii
	  mov loc3 = r9		/* preserve language specific register */
	  mov loc4 = r10	/* preserve language specific register */
	  mov loc5 = r11	/* preserve language specific register */
	}
	{ .mmi
	  stf.spill [r2] = f8, 32
	  stf.spill [r3] = f9, 32
	  mov out0 = r16
	  ;;
	}
	{ .mmi
	  stf.spill [r2] = f10, 32
	  stf.spill [r3] = f11, 32
	  shl out1 = r15, 4
	  ;;
	}
	{ .mmi
	  stf.spill [r2] = f12, 32
	  stf.spill [r3] = f13, 32
	  shladd out1 = r15, 3, out1
	  ;;
	}
	{ .mmb
	  stf.spill [r2] = f14
	  stf.spill [r3] = f15
	  br.call.sptk.many b0 = profile_fixup
	}
	{ .mii
	  ld8 r9 = [ret0], 8
	  adds r2 = 16, r12
	  adds r3 = 32, r12
	  ;;
	}
	{ .mmi
	  ldf.fill f8 = [r2], 32
	  ldf.fill f9 = [r3], 32
	  mov b0 = loc1
	  ;;
	}
	{ .mmi
	  ldf.fill f10 = [r2], 32
	  ldf.fill f11 = [r3], 32
	  mov b6 = r9
	  ;;
	}
	{ .mmi
	  ldf.fill f12 = [r2], 32
	  ldf.fill f13 = [r3], 32
	  mov ar.pfs = loc0
	  ;;
	}
	{ .mmi
	  ldf.fill f14 = [r2], 32
	  ldf.fill f15 = [r3], 32
	  .restore sp		/* pop the unwind frame state */
	  adds r12 = 160, r12
	  ;;
	}
	{ .mii
	  mov r9 = loc3		/* restore language specific register */
	  mov r10 = loc4	/* restore language specific register */
	  mov r11 = loc5	/* restore language specific register */
	}
	{ .mii
	  ld8 gp = [ret0]
	  mov r8 = loc2		/* restore struct value register */
	  ;;
	}
	/* An alloc is needed for the break system call to work.
	   We don't care about the old value of the pfs register.  */
	{ .mmb
	  .prologue
	  .body
	  alloc r2 = ar.pfs, 0, 0, 8, 0
	  br.sptk.many b6
	  ;;
	}
END (_dl_runtime_profile)
