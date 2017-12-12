#include <bits/wordsize.h>

#if __WORDSIZE == 64
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -16; ld %0, %0" : "=r" (x)); x; })
# define POINTER_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -24; ld %0, %0" : "=r" (x)); x; })
#else
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -8; ld4s %0, %0" : "=r" (x)); x; })
# define POINTER_CHK_GUARD \
  ({ uintptr_t x; asm ("addi %0, tp, -12; ld4s %0, %0" : "=r" (x)); x; })
#endif
