#include <ftw.h>
#include <getopt.h>
#include <mcheck.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


int do_depth;
int do_chdir;
int do_phys;

struct option options[] =
{
  { "depth", no_argument, &do_depth, 1 },
  { "chdir", no_argument, &do_chdir, 1 },
  { "phys", no_argument, &do_phys, 1 },
  { NULL, 0, NULL, 0 }
};

const char *flag2name[] =
{
  [FTW_F] = "FTW_F",
  [FTW_D] = "FTW_D",
  [FTW_DNR] = "FTW_DNR",
  [FTW_NS] = "FTW_NS",
  [FTW_SL] = "FTW_SL",
  [FTW_DP] = "FTW_DP",
  [FTW_SLN] = "FTW_SLN"
};


int
cb (const char *name, const struct stat *st, int flag, struct FTW *f)
{
  printf ("base = \"%.*s\", file = \"%s\", flag = %s",
	  f->base, name, name + f->base, flag2name[flag]);
  if (do_chdir)
    {
      char *cwd = getcwd (NULL, 0);
      printf (", cwd = %s", cwd);
      free (cwd);
    }
  puts ("");
  return 0;
}

int
main (int argc, char *argv[])
{
  int opt;
  int r;
  int flag = 0;
  mtrace ();

  while ((opt = getopt_long_only (argc, argv, "", options, NULL)) != -1)
    ;

  if (do_chdir)
    flag |= FTW_CHDIR;
  if (do_depth)
    flag |= FTW_DEPTH;
  if (do_phys)
    flag |= FTW_PHYS;

  r = nftw (optind < argc ? argv[optind] : ".", cb, 3, flag);
  if (r)
    perror ("nftw");
  return r;
}
