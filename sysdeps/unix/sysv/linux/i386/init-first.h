/* This fragment is invoked in the stack context of program start.
   Its job is to set up a pointer to argc as an argument, pass
   control to `INIT', and, if necessary, clean up after the call
   to leave the stack in the same condition it was found in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)	\
    asm(".globl " #NAME "\n\t"		\
	#NAME ":\n\t"			\
	"lea 4(%esp), %eax\n\t"		\
	"pushl %eax\n\t"		\
	"call " #INIT "\n\t"		\
	"popl %eax\n\t"			\
	"ret");
