/* Regression test for fseek and freopen bugs.  */

#include <stdio.h>

int
main (int argc, char *argv[])
{
  int lose = 0;
  char filename[L_tmpnam];
  FILE *fp;

  if (tmpnam (filename) == NULL)
    {
      printf ("tmpnam failed\n");
      lose = 1;
    }
  else
    {
      fp = fopen (filename, "w+");
      fprintf (fp, "Hello world!\n");
      fflush (fp);
      fseek (fp, 5L, SEEK_SET);
      if (fseek (fp, -1L, SEEK_CUR) < 0)
	{
	  printf ("seek failed\n");
	  lose = 1;
	}
      fclose (fp);
      remove (filename);
    }

  {
    FILE *file1;
    FILE *file2;
    char filename1[L_tmpnam];
    char filename2[L_tmpnam];
    int ch;

    if (tmpnam (filename1) == NULL || tmpnam (filename2) == NULL)
      {
	printf ("tmpnam failed\n");
	lose = 1;
      }
    else
      {

	file1 = fopen (filename1, "w");
	fclose (file1);

	file2 = fopen (filename2, "w");
	fputc ('x', file2);
	fclose (file2);

	file1 = fopen (filename1, "r");
	file2 = freopen (filename2, "r", file1);
	if ((ch = fgetc (file2)) != 'x')
	  {
	    printf ("wrong character in reopened file, value = %d\n", ch);
	    lose = 1;
	  }
	fclose (file2);
	remove (filename1);
	remove (filename2);
      }
  }

  puts (lose ? "Test FAILED!" : "Test succeeded.");
  return lose;
}
