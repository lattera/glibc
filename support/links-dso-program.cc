#include <iostream>

using namespace std;

int
main (int argc, char **argv)
{
  /* Complexity to keep gcc from optimizing this away.  */
  cout << (argc > 1 ? argv[1] : "null");
  return 0;
}
