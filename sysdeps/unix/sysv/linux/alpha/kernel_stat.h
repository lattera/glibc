/* Definition of `struct stat' used in the kernel.  */
struct kernel_stat
  {
    unsigned int st_dev;
    unsigned int st_ino;
    unsigned int st_mode;
    unsigned int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    long int st_size;
    unsigned long int st_atime;
    unsigned long int st_mtime;
    unsigned long int st_ctime;
    unsigned int st_blksize;
    int st_blocks;
    unsigned int st_flags;
    unsigned int st_gen;
  };

/* Definition of `struct stat' used by glibc 2.0.  */
struct glibc2_stat
  {
    __dev_t st_dev;
    __ino_t st_ino;
    __mode_t st_mode;
    __nlink_t st_nlink;
    __uid_t st_uid;
    __gid_t st_gid;
    __dev_t st_rdev;
    __off_t st_size;
    __time_t st_atime;
    __time_t st_mtime;
    __time_t st_ctime;
    unsigned int st_blksize;
    int st_blocks;
    unsigned int st_flags;
    unsigned int st_gen;
  };

#define XSTAT_IS_XSTAT64 1
