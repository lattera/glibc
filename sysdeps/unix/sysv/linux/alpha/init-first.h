/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#define SYSDEP_CALL_INIT(NAME, INIT) asm("\
	.weak _dl_starting_up
	.globl " #NAME "
	.ent " #NAME "
" #NAME ":
	ldgp	$29, 0($27)
	.prologue 1
	.set at
	/* Are we a dynamic libc being loaded into a static program?  */
	lda	$0, _dl_starting_up
	beq	$0, 1f
	ldl	$0, 0($0)
	cmpeq	$31, $0, $0
1:	stl	$0, __libc_multiple_libcs
	/* If so, argc et al are in a0-a2 already.  Otherwise, load them.  */
	bne	$0, 2f
	ldl	$16, 0($30)
	lda	$17, 8($30)
	s8addq	$16, $17, $18
	addq	$18, 8, $18
2:	br $31, " ASM_ALPHA_NG_SYMBOL_PREFIX #INIT "..ng
	.set noat
	.end " #NAME);
