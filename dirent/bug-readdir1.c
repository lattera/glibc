#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


int
main (void)
{
  DIR *dirp;
  struct dirent* ent;

  /* open a dir stream */
  dirp = opendir ("/tmp");
  if (dirp == NULL)
    {
      if (errno == ENOENT)
	exit (0);

      perror ("opendir");
      exit (1);
    }

  /* close the dir stream, making it invalid */
  if (closedir (dirp))
    {
      perror ("closedir");
      exit (1);
    }

  ent = readdir (dirp);

  return ent != NULL || errno != EBADF;
}
