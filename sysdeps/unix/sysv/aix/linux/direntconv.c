#include <dirent.h>
#include <string.h>
#include "linux-dirent.h"

#ifndef DT_UNKNOWN
# define DT_UNKNOWN 0
#endif


void
__dirent_aix_to_linux (const struct dirent *aixdir,
		       struct linuxdirent *linuxdir)
{
  linuxdir->d_ino = aixdir->d_ino;
  linuxdir->d_off = aixdir->d_off;
  linuxdir->d_reclen = aixdir->d_reclen;
  linuxdir->d_type = DT_UNKNOWN;
  strncpy (linuxdir->d_name, aixdir->d_name, 256);
}


void
__dirent64_aix_to_linux (const struct dirent64 *aixdir,
			 struct linuxdirent64 *linuxdir)
{
  linuxdir->d_ino = aixdir->d_ino;
  linuxdir->d_off = aixdir->d_off;
  linuxdir->d_reclen = aixdir->d_reclen;
  linuxdir->d_type = DT_UNKNOWN;
  strncpy (linuxdir->d_name, aixdir->d_name, 256);
}
