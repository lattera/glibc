
	.text

	.align 4

	.import main, code
	.import $global$, data
	.import __libc_start_main, code
	.import _fini, code
	.import _init, code

	
	
	
	.globl _start
	.export _start, ENTRY
	.type _start,@function
_start:

	.proc
	.callinfo

	/* load main */
	ldil	LP%main, %r26
	ldo	RP%main(%r26), %r26

	/* argc and argv should be in 25 and 24 */

	/* Expand the stack to store the 5th through 7th args */
	ldo	64(%sp), %sp
	
	/* void (*rtld_fini) (void) (actually the 6th arg) */
	stw	%r23, -56(%sp)
	
	/* void (*init) (void) */
	ldil	LP%_init, %r23
	ldo	RP%_init(%r23), %r23

	/* void (*fini) (void) */
	ldil	LP%_fini, %r22
	ldo	RP%_fini(%r22), %r22
	stw	%r22, -52(%sp)

	/* void *stack_end */
	stw	%sp, -60(%sp)

	/* load global */
	ldil	L%$global$, %dp
	ldo	R%$global$(%dp), %dp

	bl	__libc_start_main,%r2
	nop
	/* die horribly if it returned (it shouldn't) */
	iitlbp %r0,(%r0)
	nop

	.procend

