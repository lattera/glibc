extern int obj1func2 (int);

int
obj1func1 (int a)
{
  return 42 + obj1func2 (a);
}
