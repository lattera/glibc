/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

int
__execve (const char *path, char *const argv[], char *const envp[])
{
  return execve (path, argv, envp);
}
