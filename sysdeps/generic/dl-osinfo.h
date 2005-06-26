#include <stdint.h>

static inline uintptr_t __attribute__ ((always_inline))
_dl_setup_stack_chk_guard (void)
{
  uintptr_t ret = 0;
  unsigned char *p = (unsigned char *) &ret;
  p[sizeof (ret) - 1] = 255;
  p[sizeof (ret) - 2] = '\n';
  p[0] = 0;
  return ret;
}
