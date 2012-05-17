#include <bits/wordsize.h>

#ifdef __tilegx__
# if __WORDSIZE == 64
#  define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -16; ld %0, %0" : "=r" (x)); x; })
# else
#  define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -8; ld4s %0, %0" : "=r" (x)); x; })
# endif
#else
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -8; lw %0, %0" : "=r" (x)); x; })
#endif
