#include <sgidefs.h>
/* As tempting as it is to define XSTAT_IS_XSTAT64 for n64, the
   userland data structures are not identical, because of different
   padding.  */
/* Definition of `struct stat' used in the kernel.  */
#if _MIPS_SIM != _ABIO32
struct kernel_stat
  {
    unsigned int st_dev;
    unsigned int __pad1[3];
    unsigned long long st_ino;
    unsigned int st_mode;
    unsigned int st_nlink;
    int st_uid;
    int st_gid;
    unsigned int st_rdev;
    unsigned int __pad2[3];
    long long st_size;
    unsigned int st_atime;
    unsigned int __unused1;
    unsigned int st_mtime;
    unsigned int __unused2;
    unsigned int st_ctime;
    unsigned int __unused3;
    unsigned int st_blksize;
    unsigned int __pad3;
    unsigned long long st_blocks;
  };
#else
struct kernel_stat
  {
    unsigned long int st_dev;
    long int __pad1[3];			/* Reserved for network id */
    unsigned long int st_ino;
    unsigned long int st_mode;
    unsigned long int st_nlink;
    long int st_uid;
    long int st_gid;
    unsigned long int st_rdev;
    long int __pad2[2];
    long int st_size;
    long int __pad3;
    long int st_atime;
    long int __unused1;
    long int st_mtime;
    long int __unused2;
    long int st_ctime;
    long int __unused3;
    long int st_blksize;
    long int st_blocks;
    char st_fstype[16];			/* Filesystem type name, unsupported */
    long st_pad4[8];
    /* Linux specific fields */
    unsigned int st_flags;
    unsigned int st_gen;
  };
#endif
