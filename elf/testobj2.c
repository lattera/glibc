int
obj2func1 (int a __attribute__ ((unused)))
{
  return 43;
}

int
obj2func2 (int a)
{
  return obj1func1 (a) + 10;
}
