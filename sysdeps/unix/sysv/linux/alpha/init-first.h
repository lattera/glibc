/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)		\
    asm(".weak _dl_starting_up\n\t"		\
        ".globl " #NAME "\n\t"			\
	".ent " #NAME "\n"			\
	#NAME ":\n\t"				\
	"ldgp	$29, 0($27)\n\t"		\
	".prologue 1\n\t"			\
	".set at\n\t"				\
	/* Are we a dynamic libc being loaded into a static program?  */ \
	"lda	$0, _dl_starting_up\n\t"	\
	"beq	$0, 1f\n\t"			\
	"ldl	$0, 0($0)\n"			\
	"cmpeq	$31, $0, $0\n"			\
	"1:\t"					\
	"stl	$0, __libc_multiple_libcs\n\t"	\
	/* If so, argc et al are in a0-a2 already.  Otherwise, load them.  */ \
	"bne	$0, 2f\n\t"			\
	"ldl	$16, 0($30)\n\t"		\
	"lda	$17, 8($30)\n\t"		\
	"s8addq	$16, $17, $18\n\t"		\
	"addq	$18, 8, $18\n"			\
	"2:\t"					\
	"br $31, " #INIT "..ng\n\t"		\
	".set noat\n\t"				\
	".end " #NAME);
