/* Userland trampoline code for sigaction on Linux/SPARC        */
/* (C) 1996, 1997 Free Software Foundation, Inc.                */
/* This file is part of the GNU C Library.                      */
/* Contributed by Miguel de Icaza (miguel@gnu.ai.mit.edu)       */
/* Many thanks go to David Miller for explaining all this to me */
/* miguel@nuclecu.unam.mx                                       */
/* Sources:     David Miller, 4.4BSD/SPARC code                */

#include <sysdep.h>
#define __ASSEMBLY__
/* For REGWIN_SZ */
#include <asm/ptrace.h>
#include <asm/psr.h>

/* The C compiler frame size */
#define CCFSZ  96

	.text

ENTRY(____sparc_signal_trampoline)
	.global C_SYMBOL_NAME(____sig_table)

	/* Make room for 32 %f registers + %fsr
	 * this is 132 bytes + alignement = 136
	 * 96 is the C frame size
	 */
	save    %sp,-136-CCFSZ,%sp

	/* save regular registers */
	mov	%g2,%l2
	mov	%g3,%l3
	mov	%g4,%l4
	mov	%g5,%l5
	mov	%g6,%l6
	mov	%g7,%l7

       /* save fpu registers */
	ld	[%fp+64+16+20],%l0	/* load the psr from sigcontext */
	sethi	%hi(PSR_EF),%l1
	andcc	%l0,%l1,%l0		/* is floating point enabled?   */
	be	1f
	rd	%y,%l1			/* save y anyways */

	/* save fpu registers */
	st	%fsr, [%sp + CCFSZ + 0]
	std	%f0, [%sp + CCFSZ + 8]
	std	%f2, [%sp + CCFSZ + 16]
	std	%f4, [%sp + CCFSZ + 24]
	std	%f6, [%sp + CCFSZ + 32]
	std	%f8, [%sp + CCFSZ + 40]
	std	%f10, [%sp + CCFSZ + 48]
	std	%f12, [%sp + CCFSZ + 56]
	std	%f14, [%sp + CCFSZ + 64]
	std	%f16, [%sp + CCFSZ + 72]
	std	%f18, [%sp + CCFSZ + 80]
	std	%f20, [%sp + CCFSZ + 88]
	std	%f22, [%sp + CCFSZ + 96]
	std	%f24, [%sp + CCFSZ + 104]
	std	%f26, [%sp + CCFSZ + 112]
	std	%f28, [%sp + CCFSZ + 120]
	std	%f30, [%sp + CCFSZ + 128]

1:
	/* Load signal number */
	ld	[%fp + REGWIN_SZ],%o0
	mov	%fp,%o1
	mov	0xfea,%o2

	/* Sanity check */
	cmp	%o0,33
	bl	1f
	or	%g0,%g0,%g1     /*Call sys_setup */
	t	0x10
1:
#ifdef __PIC__
	/* Save return address */
	mov	%o7,%o4
___sxx:
	call	___syy
	nop
___syy:
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(___sxx-.)),%o5
	or	%o5,%lo(_GLOBAL_OFFSET_TABLE_-(___sxx-.)),%o5
	add	%o7,%o5,%o5
	/* restore return address */
	mov	%o4,%o7
	mov	%o5,%o4
	/* o4 has the GOT pointer */
#endif
	sethi	%hi(C_SYMBOL_NAME(____sig_table)),%o5
	or	%o5,%lo(C_SYMBOL_NAME(____sig_table)),%o5
#ifdef __PIC__
	add	%o5,%o4,%o4
	ld	[%o4],%o5
#endif
	sll	%o0,2,%o4
	add	%o5,%o4,%o4
	ld	[%o4],%o4
	ld	[%fp + REGWIN_SZ + 4],%o1         /* Load subcode */
	ld	[%fp + REGWIN_SZ + 8],%o2         /* pointer to sigcontext */
	call	%o4
	ld	[%fp + REGWIN_SZ + 12],%o3        /* Address where signal ocurre
 */

	/* handler returned, restore state */
	tst	%l0
	be	1f
	wr	%l1,%g0,%y

	/* fpu restoration */
	ld	[%sp + CCFSZ + 0], %fsr
	ldd	[%sp + CCFSZ + 8], %f0
	ldd	[%sp + CCFSZ + 16], %f2
	ldd	[%sp + CCFSZ + 24], %f4
	ldd	[%sp + CCFSZ + 32], %f6
	ldd	[%sp + CCFSZ + 40], %f8
	ldd	[%sp + CCFSZ + 48], %f10
	ldd	[%sp + CCFSZ + 56], %f12
	ldd	[%sp + CCFSZ + 64], %f14
	ldd	[%sp + CCFSZ + 72], %f16
	ldd	[%sp + CCFSZ + 80], %f18
	ldd	[%sp + CCFSZ + 88], %f20
	ldd	[%sp + CCFSZ + 96], %f22
	ldd	[%sp + CCFSZ + 104], %f24
	ldd	[%sp + CCFSZ + 112], %f26
	ldd	[%sp + CCFSZ + 120], %f28
	ldd	[%sp + CCFSZ + 128], %f30

1:
	mov	%l2,%g2
	mov	%l3,%g3
	mov	%l4,%g4
	mov	%l5,%g5
	mov	%l6,%g6
	mov	%l7,%g7

       /* call sigreturn */
	restore	%g0,SYS_ify(sigreturn),%g1	/* register back and set syscal */
	add	%sp,64+16,%o0
	t	0x10
	/* if we return, sysreturn failed */
	mov	SYS_ify(exit),%g1
	t	0x10
