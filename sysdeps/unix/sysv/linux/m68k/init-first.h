/* This fragment is invoked in the stack context of program start.
   Its job is to set up a pointer to argc as an argument, pass
   control to `INIT', and, if necessary, clean up after the call
   to leave the stack in the same condition it was found in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)	\
    asm(".globl " #NAME "\n\t"		\
	#NAME ":\n\t"			\
	"pea %sp@(4)\n\t"		\
	"jbsr " #INIT "\n\t"		\
	"addq #4,%sp\n\t"		\
	"rts");
