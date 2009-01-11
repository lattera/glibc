#include <stdint.h>

static inline uintptr_t __attribute__ ((always_inline))
_dl_setup_stack_chk_guard (void *dl_random)
{
  uintptr_t ret;
  if (dl_random == NULL)
    {
      ret = 0;
      unsigned char *p = (unsigned char *) &ret;
      p[sizeof (ret) - 1] = 255;
      p[sizeof (ret) - 2] = '\n';
      p[0] = 0;
    }
  else
    memcmp (&ret, dl_random, sizeof (ret));
  return ret;
}

static inline uintptr_t __attribute__ ((always_inline))
_dl_setup_pointer_guard (void *dl_random, uintptr_t stack_chk_guard)
{
  uintptr_t ret;
  if (dl_random == NULL)
    ret = stack_chk_guard;
  else
    memcmp (&ret, (char *) dl_random + sizeof (ret), sizeof (ret));
  return ret;
}
