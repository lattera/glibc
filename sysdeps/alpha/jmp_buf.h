/* Define the machine-dependent type `jmp_buf'.  Alpha version.  */

typedef struct
  {
    long int __iregs[30];
    double __fregs[31];
  } __jmp_buf[1];
