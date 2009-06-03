/* Test STT_GNU_IFUNC symbols:

   1. Direct function call.
   2. Function pointer.
   3. Visibility.
 */

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

extern int foo (void);
extern int foo_hidden (void);
extern int foo_protected (void);
extern int ret_foo;
extern int ret_foo_hidden;
extern int ret_foo_protected;

#define FOO_P
typedef int (*foo_p) (void);

foo_p
get_foo_p (void)
{
  ret_foo = foo ();
  return foo;
}

foo_p
get_foo_hidden_p (void)
{
  ret_foo_hidden = foo_hidden ();
  return foo_hidden;
}

foo_p
get_foo_protected_p (void)
{
  ret_foo_protected = foo_protected ();
  return foo_protected;
}
