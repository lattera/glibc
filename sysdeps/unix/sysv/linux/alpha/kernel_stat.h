/* Definition of `struct stat' used in the kernel..  */
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
