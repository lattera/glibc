#include <ansidecl.h>
#include <stdio.h>

#define TESTFILE "test.dat"

int
main __P((void))
{
  FILE *fp;
  int i, j;

  puts ("\nFile seek test");
  fp = fopen (TESTFILE, "w");
  if (fp == NULL)
    {
      perror (TESTFILE);
      return 1;
    }

  for (i = 0; i < 256; i++)
    putc (i, fp);
  if (freopen (TESTFILE, "r", fp) != fp)
    {
      perror ("Cannot open file for reading");
      return 1;
    }

  for (i = 1; i <= 255; i++)
    {
      printf ("%3d\n", i);
      fseek (fp, (long) -i, SEEK_END);
      if ((j = getc (fp)) != 256 - i)
	{
	  printf ("SEEK_END failed %d\n", j);
	  break;
	}
      if (fseek (fp, (long) i, SEEK_SET))
	{
	  puts ("Cannot SEEK_SET");
	  break;
	}
      if ((j = getc (fp)) != i)
	{
	  printf ("SEEK_SET failed %d\n", j);
	  break;
	}
      if (fseek (fp, (long) i, SEEK_SET))
	{
	  puts ("Cannot SEEK_SET");
	  break;
	}
      if (fseek (fp, (long) (i >= 128 ? -128 : 128), SEEK_CUR))
	{
	  puts ("Cannot SEEK_CUR");
	  break;
	}
      if ((j = getc (fp)) != (i >= 128 ? i - 128 : i + 128))
	{
	  printf ("SEEK_CUR failed %d\n", j);
	  break;
	}
    }
  fclose (fp);

  puts ((i > 255) ? "Test succeeded." : "Test FAILED!");
  return (i > 255) ? 0 : 1;
}
