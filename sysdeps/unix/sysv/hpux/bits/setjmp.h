/* Define the machine-dependent type `jmp_buf'.  Stub version.  */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* XXX This should go into different files!!! */

#ifdef __hp9000s300
typedef int __jmp_buf[100];
#endif /* __hp9000s300 */

#ifdef __hp9000s800
typedef double __jmp_buf[25];
#endif /* __hp9000s800 */

