extern int baz (void);

int
bar (void)
{
  return -21 + baz ();
}
