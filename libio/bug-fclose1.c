// BZ #12724

static void do_prepare (void);
#define PREPARE(argc, argv) do_prepare ()
static int do_test (void);
#define TEST_FUNCTION do_test()
#include "../test-skeleton.c"


static int fd;


static void
do_prepare (void)
{
  fd = create_temp_file ("bug-fclose1.", NULL);
  if (fd == -1)
    {
      printf ("cannot create temporary file: %m\n");
      exit (1);
    }
}


static int
do_test (void)
{
  static const char pattern[] = "hello world";

  /* Prepare a seekable file.  */
  if (write (fd, pattern, sizeof pattern) != sizeof pattern)
    {
      printf ("cannot write pattern: %m\n");
      return 1;
    }
  if (lseek (fd, 1, SEEK_SET) != 1)
    {
      printf ("cannot seek after write: %m\n");
      return 1;
    }

  /* Create an output stream visiting the file; when it is closed, all
     other file descriptors visiting the file must see the new file
     position.  */
  int fd2 = dup (fd);
  if (fd2 < 0)
    {
      printf ("cannot duplicate descriptor for writing: %m\n");
      return 1;
    }
  FILE *f = fdopen (fd2, "w");
  if (f == NULL)
    {
      printf ("first fdopen failed: %m\n");
      return 1;
    }
  if (fputc (pattern[1], f) != pattern[1])
    {
      printf ("fputc failed: %m\n");
      return 1;
    }
  if (fclose (f) != 0)
    {
      printf ("first fclose failed: %m\n");
      return 1;
    }
  errno = 0;
  if (lseek (fd2, 0, SEEK_CUR) != -1)
    {
      printf ("lseek after fclose after write did not fail\n");
      return 1;
    }
  if (errno != EBADF)
    {
      printf ("lseek after fclose after write did not fail with EBADF: %m\n");
      return 1;
    }
  off_t o = lseek (fd, 0, SEEK_CUR);
  if (o != 2)
    {
      printf ("\
lseek on original descriptor after first fclose returned %ld, expected 2\n",
	      (long int) o);
      return 1;
    }

  /* Likewise for an input stream.  */
  fd2 = dup (fd);
  if (fd2 < 0)
     {
      printf ("cannot duplicate descriptor for reading: %m\n");
      return 1;
    }
  f = fdopen (fd2, "r");
   if (f == NULL)
    {
      printf ("second fdopen failed: %m\n");
      return 1;
    }
   char c = fgetc (f);
   if (c != pattern[2])
     {
       printf ("getc returned %c, expected %c\n", c, pattern[2]);
       return 1;
     }
  if (fclose (f) != 0)
    {
      printf ("second fclose failed: %m\n");
      return 1;
    }
  errno = 0;
  if (lseek (fd2, 0, SEEK_CUR) != -1)
    {
      printf ("lseek after fclose after read did not fail\n");
      return 1;
    }
  if (errno != EBADF)
    {
      printf ("lseek after fclose after read did not fail with EBADF: %m\n");
      return 1;
    }
  o = lseek (fd, 0, SEEK_CUR);
  if (o != 3)
    {
      printf ("\
lseek on original descriptor after second fclose returned %ld, expected 3\n",
	      (long int) o);
      return 1;
    }

  return 0;
}
