/*
 * Sparc v8 has divide.
 */

#include <sysdep.h>

ENTRY(.div)

	sra	%o0, 31, %o2
	wr	%o2, 0, %y
	ret
	 sdiv	%o0, %o1, %o0

END(.div)
