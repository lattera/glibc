#include <unistd.h>

extern int accessx (const char *name, int type, int who);

int
__access (const char *name, int type)
{
  return accessx (name, type, ACC_INVOKER);
}
strong_alias (__access, access)
