#include <dlfcn.h>

int main(void)
{
  void *h = dlopen("$ORIGIN/tst-auditmod9b.so", RTLD_LAZY);
  int (*fp)(void) = dlsym(h, "f");
  return fp() - 1;
}
