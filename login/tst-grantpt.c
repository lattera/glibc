#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static int
test_ebadf (void)
{
  int fd, ret, err;

  fd = posix_openpt (O_RDWR);
  if (fd == -1)
    {
      printf ("posix_openpt(O_RDWR) failed\nerrno %d (%s)\n",
	      errno, strerror (errno));
      /* We don't fail because of this; maybe the system does not have
	 SUS pseudo terminals.  */
      return 0;
    }
  unlockpt (fd);
  close (fd);

  ret = grantpt (fd);
  err = errno;
  if (ret != -1 || err != EBADF)
    {
      printf ("grantpt(): expected: return = %d, errno = %d\n", -1, EBADF);
      printf ("           got: return = %d, errno = %d\n", ret, err);
      return 1;
    }
  return 0;
}

static int
test_einval (void)
{
  int fd, ret, err;
  const char file[] = "./grantpt-einval";

  fd = open (file, O_RDWR | O_CREAT, 0600);
  if (fd == -1)
    {
      printf ("open(\"%s\", O_RDWR) failed\nerrno %d (%s)\n",
	      file, errno, strerror (errno));
      return 0;
    }
  unlink (file);

  ret = grantpt (fd);
  err = errno;
  if (ret != -1 || err != EINVAL)
    {
      printf ("grantpt(): expected: return = %d, errno = %d\n", -1, EINVAL);
      printf ("           got: return = %d, errno = %d\n", ret, err);
      ret = 1;
    }
  else
    ret = 0;

  close (fd);

  return ret;
}

static int
do_test (void)
{
  int result = 0;

  result += test_ebadf ();
  result += test_einval ();

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
