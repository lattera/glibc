/* Define the machine-dependent type `jmp_buf'.  PowerPC version.  */

/* The previous jmp_buf.h had __jmp_buf defined as a structure.
   We use an array of 'long int' instead, to make writing the
   assembler easier. Naturally, user code should not depend on
   either representation. */

#if	defined (__USE_MISC) || defined (_ASM)
#define JB_GPR1  0   /* also known as the stack pointer */
#define JB_GPR2  1
#define JB_LR    2
#define JB_GPRS  3   /* GPRs 14 through 31 are saved, 18 in total */
#define JB_UNUSED 21 /* it's sometimes faster to store doubles word-aligned */
#define JB_FPRS  22  /* FPRs 14 through 31 are saved, 18*2 words total */
#endif

#ifndef	_ASM
typedef long int __jmp_buf[58];
#endif
