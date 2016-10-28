#include <unistd.h>
#include <string.h>

static void
write_message (const char *message)
{
  ssize_t unused __attribute__ ((unused));
  unused = write (STDOUT_FILENO, message, strlen (message));
}

struct statclass
{
  statclass()
  {
    write_message ("statclass\n");
  }
  ~statclass()
  {
    write_message ("~statclass\n");
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
