/* Define the machine-dependent type `jmp_buf'.  Intel 386 version.  */

#if	defined (__USE_MISC) || defined (_ASM)
#define	JB_BX	0
#define	JB_SI	1
#define	JB_DI	2
#define	JB_BP	3
#define	JB_SP	4
#define	JB_PC	5
#endif

#ifndef	_ASM
typedef int __jmp_buf[6];
#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((int) (address) < (jmpbuf)[JB_SP])
