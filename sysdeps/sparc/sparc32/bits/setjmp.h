/* Define the machine-dependent type `jmp_buf'.  SPARC version.  */

#if	defined (__USE_MISC) || defined (_ASM)
#define	JB_SP	0
#define	JB_FP	1
#define	JB_PC	2
#endif

#ifndef	_ASM
typedef int __jmp_buf[3];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((int) (address) < (jmpbuf)[JB_SP])
