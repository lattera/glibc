/* Define the machine-dependent type `jmp_buf'.  SPARC version.  */

/* NOTE: The assembly code in __longjmp.S and setjmp.S knows the layout
   of this structure.  You must hack the assembly code if you want to change
   the order of the members.  */

typedef struct
  {
    /* Return PC (register o7).  */
    __ptr_t __pc;

    /* Saved FP.  */
    __ptr_t __fp;
  } __jmp_buf[1];
