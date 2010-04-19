#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int
main (int argc, char **argv)
{
  const char *test_file;
  const char blah[] = "BLAH";
  FILE *fp;
  char *mmap_data;
  int ch, fd;
  struct stat fs;
  const char *cp;

  /* Construct the test file name based on ARGV[0], which will be
     an absolute file name in the build directory.  Don't touch the
     source directory, which might be read-only.  */
  if (argc != 1 || asprintf (&test_file, "%s.test", argv[0]) < 0)
    exit (99);

  /* setup the physical file, and use it */
  if ((fp = fopen (test_file, "w+")) == NULL)
    exit (1);
  if (fwrite (blah, 1, strlen (blah), fp) != strlen (blah))
    exit (2);

  rewind (fp);
  printf ("file: ");
  cp = blah;
  while ((ch = getc (fp)) != EOF)
    {
      fputc (ch, stdout);
      if (ch != *cp)
	{
	  printf ("\ncharacter %td: '%c' instead of '%c'\n",
		  cp - blah, ch, *cp);
	  exit (1);
	}
      ++cp;
    }
  fputc ('\n', stdout);
  if (ferror (fp))
    {
      puts ("fp: error");
      exit (1);
    }
  if (feof (fp))
    printf ("fp: EOF\n");
  else
    {
      puts ("not EOF");
      exit (1);
    }
  fclose (fp);

  /* Now, mmap the file into a buffer, and do that too */
  if ((fd = open (test_file, O_RDONLY)) == -1)
    exit (3);
  if (fstat (fd, &fs) == -1)
    exit (4);

  if ((mmap_data = (char *) mmap (NULL, fs.st_size, PROT_READ,
				  MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
      if (errno == ENOSYS)
	exit (0);
      exit (5);
    }

  if ((fp = fmemopen (mmap_data, fs.st_size, "r")) == NULL)
    exit (1);

  printf ("mem: ");
  cp = blah;
  while ((ch = getc (fp)) != EOF)
    {
      fputc (ch, stdout);
      if (ch != *cp)
	{
	  printf ("%td character: '%c' instead of '%c'\n",
		  cp - blah, ch, *cp);
	  exit (1);
	}
      ++cp;
    }

  fputc ('\n', stdout);

  if (ferror (fp))
    {
      puts ("fp: error");
      exit (1);
    }
  if (feof (fp))
    printf ("fp: EOF\n");
  else
    {
      puts ("not EOF");
      exit (1);
    }

  fclose (fp);

  munmap (mmap_data, fs.st_size);

  unlink (test_file);
  free (test_file);

  return 0;
}
