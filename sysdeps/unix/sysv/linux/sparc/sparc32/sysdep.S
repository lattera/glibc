/* Define errno */

	.bss
	.globl	errno
	.align	4
errno:	.space	4
	.type errno, @object
	.size errno, 4

	.globl	_errno
_errno	= errno
