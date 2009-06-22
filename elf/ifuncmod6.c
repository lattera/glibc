/* Test STT_GNU_IFUNC symbol reference in a shared library.  */

extern int foo (void);

typedef int (*foo_p) (void);

foo_p foo_ptr = foo;

foo_p
get_foo_p (void)
{
  return foo_ptr;
}

foo_p
get_foo (void)
{
  return foo;
}
