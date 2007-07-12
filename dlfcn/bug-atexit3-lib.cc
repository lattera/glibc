#include <unistd.h>

struct statclass
{
  statclass()
  {
    write (1, "statclass\n", 10);
  }
  ~statclass()
  {
    write (1, "~statclass\n", 11);
  }
};

struct extclass
{
  ~extclass()
  {
    static statclass var;
  }
};

extclass globvar;
