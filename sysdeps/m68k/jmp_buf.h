/* Define the machine-dependent type `jmp_buf'.  m68k version.  */

typedef struct
  {
    /* There are eight 4-byte data registers, but D0 is not saved.  */
    long int __dregs[7];

    /* There are six 4-byte address registers, plus the FP and SP.  */
    int *__aregs[6];
    int * __fp;
    int * __sp;

#if defined(__HAVE_68881__) || defined(__HAVE_FPU__)
    /* There are eight floating point registers which
       are saved in IEEE 96-bit extended format.  */
    char __fpregs[8 * (96 / 8)];
#endif

  } __jmp_buf[1];
