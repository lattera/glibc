/* Test STT_GNU_IFUNC symbols without direct function call.  */

extern int global;

static int
one (void)
{
  return 1;
}

static int
minus_one (void)
{
  return -1;
}

static int
zero (void)
{
  return 0;
}

void * foo_ifunc (void) __asm__ ("foo");
__asm__(".type foo, %gnu_indirect_function");

void *
foo_ifunc (void)
{
  switch (global)
    {
    case 1:
      return one;
    case -1:
      return minus_one;
    default:
      return zero;
    }
}

void * foo_hidden_ifunc (void) __asm__ ("foo_hidden");
__asm__(".type foo_hidden, %gnu_indirect_function");

void *
foo_hidden_ifunc (void)
{
  switch (global)
    {
    case 1:
      return minus_one;
    case -1:
      return one;
    default:
      return zero;
    }
}

void * foo_protected_ifunc (void) __asm__ ("foo_protected");
__asm__(".type foo_protected, %gnu_indirect_function");

void *
foo_protected_ifunc (void)
{
  switch (global)
    {
    case 1:
      return one;
    case -1:
      return zero;
    default:
      return minus_one;
    }
}

/* Test hidden indirect function.  */
__asm__(".hidden foo_hidden");

/* Test protected indirect function.  */
__asm__(".protected foo_protected");
