#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int
cb (const char *fname, const struct stat *st, int flag)
{
  printf ("%s %d\n", fname, flag);
  return 0;
}

int
main (void)
{
  char tmp[] = "/tmp/ftwXXXXXX";
  char *dname;
  int r;
  int e;

  dname = mkdtemp (tmp);
  if (dname == NULL)
    {
      printf ("mkdtemp: %m\n");
      exit (1);
    }

  if (chmod (dname, S_IWUSR|S_IXUSR|S_IWGRP|S_IXGRP|S_IWOTH|S_IXOTH) != 0)
    {
      printf ("chmod: %m\n");
      exit (1);
    }

  r = ftw (dname, cb, 10);
  e = errno;
  printf ("r = %d", r);
  if (r != 0)
    printf (", errno = %d", errno);
  puts ("");

  chmod (dname, S_IRWXU|S_IRWXG|S_IRWXO);
  rmdir (dname);

  return r != -1 && e == EACCES;
}
