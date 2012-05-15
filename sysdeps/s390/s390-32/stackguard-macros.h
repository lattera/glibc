#include <stdint.h>

#define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ear %0,%%a0; l %0,0x14(%0)" : "=a" (x)); x; })
