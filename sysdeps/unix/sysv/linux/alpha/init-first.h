/* This fragment is invoked in the stack context of program start.
   Its job is to set up a pointer to argc as an argument, pass
   control to `INIT', and, if necessary, clean up after the call
   to leave the stack in the same condition it was found in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)	\
    asm(".globl " #NAME "\n"		\
	#NAME ":\n\t"			\
	"ldgp $29, 0($27)\n\t"		\
	".prologue 1\n\t"		\
	"mov $30, $16\n\t"		\
	"br $31, " #INIT "..ng");
