/* Test case by Horst von Brand <vonbrand@sleipnir.valparaiso.cl>.  */
#include <mntent.h>
#include <stdio.h>
#include <string.h>


int
main (int argc, char *argv[])
{
  int result = 0;
  struct mntent mef;
  struct mntent *mnt = &mef;

  mef.mnt_fsname = strdupa ("/dev/hda1");
  mef.mnt_dir = strdupa ("/");
  mef.mnt_type = strdupa ("ext2");
  mef.mnt_opts = strdupa ("defaults");
  mef.mnt_freq = 1;
  mef.mnt_passno = 1;

  if (hasmntopt (mnt, "defaults"))
    printf ("Found!\n");
  else
    {
      printf ("Didn't find it\n");
      result = 1;
    }

  return result;
}
