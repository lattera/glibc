extern int bar (void);

int
foo (void)
{
  return 10 + bar ();
}
