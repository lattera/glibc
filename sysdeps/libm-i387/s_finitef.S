/*
 * Written by Joe Keane <jgk@jgk.org>.
 */

#include <machine/asm.h>

ENTRY(__finitef)
	movl	4(%esp),%eax
	orl	$0x807fffff, %eax
	incl	%eax
	shrl	$31, %eax
	ret
END (__finitef)
weak_alias (__finitef, finitef)
