/* Define the machine-dependent type `jmp_buf'.  ARM version. */

#ifndef _ASM
/* Jump buffer contains v1-v6, sl, fp, sp, pc and (f4-f7) if we do FP. */
#if __ARM_USES_FP
typedef int __jmp_buf[22];
#else
typedef int __jmp_buf[10];
#endif
#endif
