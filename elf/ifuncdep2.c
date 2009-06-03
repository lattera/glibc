/* Test 3 STT_GNU_IFUNC symbols.  */

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

void * foo1_ifunc (void) __asm__ ("foo1");
__asm__(".type foo1, %gnu_indirect_function");

void * 
foo1_ifunc (void)
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

void * foo2_ifunc (void) __asm__ ("foo2");
__asm__(".type foo2, %gnu_indirect_function");

void * 
foo2_ifunc (void)
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

void * foo3_ifunc (void) __asm__ ("foo3");
__asm__(".type foo3, %gnu_indirect_function");

void * 
foo3_ifunc (void)
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
