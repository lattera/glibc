extern int foo (void);

int
call_me (void)
{
  return foo () - 42;
}
