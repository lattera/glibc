/* Define errno */

.section .bss
	.globl	errno
	.align	4
errno:	.space	4
	.type errno, @object
	.size errno, 4

weak_alias (errno, _errno)
