/* hp300 4.3 BSD starts at 4, rather than 0, when the start address is 0.
   Go figure.  */
asm(".globl __start\n"
    "__start:	.ascii \"scum\""); /* He he.  */

#define	_start	__start0

#define	DUMMIES	dummy0

#include <sysdeps/unix/start.c>
