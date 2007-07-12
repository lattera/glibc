/*  4 instruction cycles not accessing cache and TLB are needed after
    trapa instruction to avoid an SH-4 silicon bug.  */
#define NEED_SYSCALL_INST_PAD
#include_next <lowlevellock.h>
