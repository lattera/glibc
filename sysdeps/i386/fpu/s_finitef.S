/*
 * Written by Joe Keane <jgk@jgk.org>.
 */

#include <machine/asm.h>

ENTRY(__finitef)
	movl	4(%esp),%eax
	movl    $0xFF7FFFFF,%ecx
	subl    %eax,%ecx
	xorl    %ecx,%eax
	shrl    $31,%eax
	ret
END (__finitef)
strong_alias(__finitef, __finitef_internal)
weak_alias (__finitef, finitef)
