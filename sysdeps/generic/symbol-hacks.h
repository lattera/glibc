/* Some compiler optimizations may transform loops into memset/memmove
   calls and without proper declaration it may generate PLT calls.  */
#if !defined __ASSEMBLER__ && IS_IN (libc) && defined SHARED
asm ("memmove = __GI_memmove");
asm ("memset = __GI_memset");
asm ("memcpy = __GI_memcpy");
#endif
