#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

int
main ()
{

  DIR * dirp;
  long save3;
  int i = 0;
  struct dirent *dp;

  dirp = opendir(".");
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
      /* save position 3 (fourth entry) */
      if (i++ == 3)
	save3 = telldir(dirp);

      printf("%s\n", dp->d_name);

      /* stop at 400 (just to make sure dirp->__offset and dirp->__size are
	 scrambled */
      if (i == 400)
	break;
    }

  /* go back to saved entry */
  seekdir (dirp, save3);

  
  /* print remaining files (3-last) */
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    printf("%s\n", dp->d_name);


  closedir (dirp);
  exit(0);
}
