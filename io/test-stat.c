/* We need to define:
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE 1
*/

#include <assert.h>
#include <stddef.h>
#include <sys/stat.h>

int
main (void)
{
  /* With _FILE_OFFSET_BITS=64 struct stat and struct stat64 should
     be identical.  */
  assert (sizeof (struct stat)
	  == sizeof (struct stat64));
  assert (offsetof (struct stat, st_dev)
	  == offsetof (struct stat64, st_dev));
  assert (offsetof (struct stat, st_ino)
	  == offsetof (struct stat64, st_ino));
  assert (offsetof (struct stat, st_mode)
	  == offsetof (struct stat64, st_mode));
  assert (offsetof (struct stat, st_nlink)
	  == offsetof (struct stat64, st_nlink));
  assert (offsetof (struct stat, st_uid)
	  == offsetof (struct stat64, st_uid));
  assert (offsetof (struct stat, st_gid)
	  == offsetof (struct stat64, st_gid));
  assert (offsetof (struct stat, st_rdev)
	  == offsetof (struct stat64, st_rdev));
  assert (offsetof (struct stat, st_size)
	  == offsetof (struct stat64, st_size));
  assert (offsetof (struct stat, st_atime)
	  == offsetof (struct stat64, st_atime));
  assert (offsetof (struct stat, st_mtime)
	  == offsetof (struct stat64, st_mtime));
  assert (offsetof (struct stat, st_ctime)
	  == offsetof (struct stat64, st_ctime));
  assert (offsetof (struct stat, st_blksize)
	  == offsetof (struct stat64, st_blksize));
  assert (offsetof (struct stat, st_blocks)
	  == offsetof (struct stat64, st_blocks));
#if 0
  /* Some systems have st_fstype but not all.  Don't check it for now.  */
  assert (offsetof (struct stat, st_fstype)
	  == offsetof (struct stat64, st_fstype));
#endif
  return 0;
}
