/*
 * Written by Joe Keane <jgk@jgk.org>.
 */

#include <machine/asm.h>

ENTRY(__finite)
	movl	8(%esp)
	movl    $0xFFEFFFFF,%ecx
	subl    %eax,%ecx
	xorl    %ecx,%eax,%eax
	shrl	$31, %eax
	ret
END (__finite)
weak_alias (__finite, finite)
