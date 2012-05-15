#include <stdint.h>

#define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ear %0,%%a0; sllg %0,%0,32; ear %0,%%a1; lg %0,0x28(%0)" : "=a" (x)); x; })
