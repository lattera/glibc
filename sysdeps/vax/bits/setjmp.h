/* Define the machine-dependent type `jmp_buf'.  Vax version.  */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct
  {
    PTR __fp;
    PTR __pc;
  } __jmp_buf[1];
