/*
 * Written by Joe Keane <jgk@jgk.org>.
 */

#include <machine/asm.h>

ENTRY(__finite)
	movl	8(%esp),%eax
	orl	$0x800fffff, %eax
	incl	%eax
	shrl	$31, %eax
	ret
END (__finite)
weak_alias (__finite, finite)
