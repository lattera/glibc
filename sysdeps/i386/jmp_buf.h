/* Define the machine-dependent type `jmp_buf'.  Intel 386 version.  */

typedef struct
  {
    long int __bx, __si, __di;
    __ptr_t __bp;
    __ptr_t __sp;
    __ptr_t __pc;
  } __jmp_buf[1];

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((__ptr_t) (address) < (jmpbuf)[0].__sp)
