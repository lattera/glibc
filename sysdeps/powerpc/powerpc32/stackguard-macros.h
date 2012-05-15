#include <stdint.h>

#define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("lwz %0,-28680(2)" : "=r" (x)); x; })
