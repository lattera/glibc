/* This function is supposed to not exist.  */
extern int xyzzy (int);

int
foo (int a)
{
  return xyzzy (a);
}
