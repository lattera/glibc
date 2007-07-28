#include <fcntl.h>
#include <sys/stat.h>

static void do_prepare (void);
#define PREPARE(argc, argv) do_prepare ()
static int do_test (void);
#define TEST_FUNCTION do_test ()
#include <test-skeleton.c>

static int fd;

static void
do_prepare (void)
{
  fd = create_temp_file ("tst-posix_fallocate.", NULL);
  if (fd == -1)
    {
      printf ("cannot create temporary file: %m\n");
      exit (1);
    }
}


static int
do_test (void)
{
  struct stat64 st;

  if (fstat64 (fd, &st) != 0)
    {
      puts ("1st fstat failed");
      return 1;
    }

  if (st.st_size != 0)
    {
      puts ("file not created with size 0");
      return 1;
    }

  if (posix_fallocate (fd, 512, 768) != 0)
    {
      puts ("1st posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("2nd fstat failed");
      return 1;
    }

  if (st.st_size != 512 + 768)
    {
      printf ("file size after first posix_fallocate call is %llu, expected %u\n",
	      (unsigned long long int) st.st_size, 512u + 768u);
      return 1;
    }

  if (posix_fallocate (fd, 0, 1024) != 0)
    {
      puts ("2nd posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("3rd fstat failed");
      return 1;
    }

  if (st.st_size != 512 + 768)
    {
      puts ("file size changed in second posix_fallocate");
      return 1;
    }

  if (posix_fallocate (fd, 2048, 64) != 0)
    {
      puts ("3rd posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("4th fstat failed");
      return 1;
    }

  if (st.st_size != 2048 + 64)
    {
      printf ("file size after first posix_fallocate call is %llu, expected %u\n",
	      (unsigned long long int) st.st_size, 2048u + 64u);
      return 1;
    }

  close (fd);

  return 0;
}
