/* Definition of `struct stat' used in the kernel */
struct kernel_stat
  {
    unsigned int st_dev;
    unsigned long int st_ino;
    unsigned int st_mode;
    short int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    long int st_size;
    long int st_atime_sec;
    long int st_mtime_sec;
    long int st_ctime_sec;
    long int st_blksize;
    long int st_blocks;
    unsigned long int __glibc_reserved1;
    unsigned long int __glibc_reserved2;
  };

/* Definition of `struct stat64' used in the kernel.  */
struct kernel_stat64
  {
    unsigned long int st_dev;
    unsigned long int st_ino;
    unsigned long int st_nlink;

    unsigned int st_mode;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int __pad0;

    unsigned long int st_rdev;
    long int st_size;
    long int st_blksize;
    long int st_blocks;

    unsigned long int st_atime_sec;
    unsigned long int st_atime_nsec;
    unsigned long int st_mtime_sec;
    unsigned long int st_mtime_nsec;
    unsigned long int st_ctime_sec;
    unsigned long int st_ctime_nsec;
    long int __glibc_reserved[3];
  };

#define XSTAT_IS_XSTAT64 1
#define STATFS_IS_STATFS64 0
