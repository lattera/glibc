extern int bar (void);

int
baz (void)
{
  return -42;
}

int
xyzzy (void)
{
  return 10 + bar ();
}
